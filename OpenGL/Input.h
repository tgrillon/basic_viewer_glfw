#include <GLFW/glfw3.h>
#include <unordered_map>
#include <queue>
/**
 * TODO: Peux mieux faire, utiliser (int mod) pour shift/alt 
*/

struct KeyData {
  KeyData(){}
  KeyData(int key1, int key2, int key3, bool hold):
    key1(key1), key2(key2), key3(key3), hold(hold) {
      if (key1 >= 0) nb_keys++;
      if (key2 >= 0) nb_keys++;
      if (key3 >= 0) nb_keys++;
    }

  int key1 = -1, key2 = -1, key3 = -1;
  bool hold = false;
  int nb_keys = 0;

  bool operator==(const KeyData& other) const{
    return hold == other.hold && key1 == other.key1 && key2 == other.key2 && key3 == other.key3;
  }

};

template<>
struct std::hash<KeyData>{
  std::size_t operator()(const KeyData& k) const noexcept{ 
    std::size_t h1 = std::hash<int>{}(k.key1);
    std::size_t h2 = std::hash<int>{}(k.key2);
    std::size_t h3 = std::hash<int>{}(k.key3);
    std::size_t h4 = std::hash<bool>{}(k.hold);
    return h1 ^ (h2 << 1) ^ (h3 << 1) ^ (h4 << 1);
  }
};


struct Action{
  Action(){};
  Action(KeyData keys, int action): keys(keys), action(action){};

  KeyData keys; 
  int action;
};

template<>
struct std::less<Action>{
  constexpr bool operator()(const Action& lhs, const Action& rhs) const 
  {
    return lhs.keys.nb_keys > rhs.keys.nb_keys; 
  }
};

struct Cursor{
  Cursor() {};
  Cursor(double x, double y) : x(x), y(y) {};
  double x;
  double y;
};

class Input
{
public:
  using ActionEnum = int;
private:
  std::array<bool, GLFW_KEY_LAST> pressed_keys = {}, holding_keys = {}, consumed_keys = {};
  std::array<bool, GLFW_MOUSE_BUTTON_LAST> mouse_pressed = {}, mouse_hold = {}, consumed_mouse_btn = {};
  
  std::deque<Action> key_actions, mouse_actions;
  Cursor cursor;

public:
  Cursor get_cursor() { return cursor; }

  void add_action(int key, bool hold, ActionEnum action);
  void add_action(int key, int mod1, bool hold, ActionEnum action);
  void add_action(int key, int mod1, int mod2, bool hold, ActionEnum action);

  void add_mouse_action(KeyData keys, ActionEnum action);
  void add_mouse_action(int btn, bool hold, ActionEnum action);
  void add_mouse_action(int btn, int mod1, bool hold, ActionEnum action);
  void add_mouse_action(int btn, int mod1, int mod2, bool hold, ActionEnum action);

protected:
  void on_key_event(int key, int scancode, int action, int mods);
  void on_cursor_event(double xpos, double ypo);
  void on_mouse_btn_event(int button, int action, int mods);
  void handle_events();

  virtual void handle_actions(ActionEnum action) = 0;
private:
  void add_action(std::deque<Action>& actions, KeyData keys, ActionEnum action);
};

void Input::add_action(int key, int mod1, int mod2, bool hold, ActionEnum action) {
  add_action(key_actions, KeyData(key, mod1, mod2, hold), action);
};

void Input::add_action(int key, int mod1, bool hold, ActionEnum action) {
  add_action(key_actions, KeyData(key, mod1, -1, hold), action);
};

void Input::add_action(int key, bool hold, ActionEnum action) {
  add_action(key_actions, KeyData(key, -1, -1, hold), action);
};

void Input::add_mouse_action(int btn, bool hold, ActionEnum action) {
  add_action(mouse_actions, KeyData(btn, -1, -1, hold), action);
};

void Input::add_mouse_action(int btn, int mod1, bool hold, ActionEnum action) {
  add_action(mouse_actions, KeyData(btn, mod1, -1, hold), action);
};

void Input::add_mouse_action(int btn, int mod1, int mod2, bool hold, ActionEnum action) {
  add_action(mouse_actions, KeyData(btn, mod1, mod2, hold), action);
};

void Input::add_action(std::deque<Action>& actions, KeyData keys, ActionEnum action) {
  auto it = actions.begin();
  Action act(keys, action);

  while (it != actions.end()){
    if (it->keys.nb_keys < keys.nb_keys){
      actions.insert(it, act);
      return;
    }
    it++;
  }

  actions.push_back(act);
};

void Input::on_key_event(int key, int scancode, int action, int mods){
  if (action == GLFW_PRESS) {
    pressed_keys[key] = true;
    holding_keys[key] = true;
  }

  if (action == GLFW_RELEASE) {
    holding_keys[key] = false;
  }
}

void Input::on_cursor_event(double xpos, double ypo) {
  cursor = Cursor(xpos, ypo);
}

void Input::on_mouse_btn_event(int btn, int action, int mods) {
  if (action == GLFW_PRESS) {
    mouse_pressed[btn] = true;
    mouse_hold[btn] = true;
  }

  if (action == GLFW_RELEASE) {
    mouse_hold[btn] = false; 
  }
}

void Input::handle_events(){
  pressed_keys = {};
  consumed_keys = {};
  mouse_pressed = {};
  consumed_mouse_btn = {};

  glfwPollEvents();

  for (Action act : key_actions){
    KeyData k = act.keys;
    ActionEnum action = act.action;

    bool* key_array = k.hold ? holding_keys.data() : pressed_keys.data();

    if (!consumed_keys[k.key1] && key_array[k.key1]
    && (k.key2 < 0 || holding_keys[k.key2])
    && (k.key3 < 0 || holding_keys[k.key3])){
      consumed_keys[k.key1] = true;
      handle_actions(action);
    }
  }

  for (Action act : mouse_actions){
    KeyData k = act.keys;
    ActionEnum action = act.action;

    bool* btn_array = k.hold ? mouse_hold.data() : mouse_pressed.data();

    if (!consumed_mouse_btn[k.key1] && btn_array[k.key1]
    && (k.key2 < 0 || holding_keys[k.key2])
    && (k.key3 < 0 || holding_keys[k.key3])){
      consumed_keys[k.key1] = true;
      handle_actions(action);
    }
  }
};

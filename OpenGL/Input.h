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
};

struct Action{
  Action(){};
  Action(KeyData keys, int action): keys(keys), action(action){};

  KeyData keys; 
  int action;
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
  // Le bordel, pourrait être plus simple
  std::unordered_map<int, bool> pressed_keys, holding_keys, consumed_keys;
  std::unordered_map<int, bool> mouse_pressed, mouse_hold, consumed_mouse_btn;
  std::unordered_map<ActionEnum, bool> started_actions, activated_actions;

  std::deque<Action> key_actions, mouse_actions;
  Cursor cursor;

public:
  Cursor get_cursor() { return cursor; }

  void add_action(int key, bool hold, ActionEnum action);
  void add_action(int key, int mod1, bool hold, ActionEnum action);
  void add_action(int key, int mod1, int mod2, bool hold, ActionEnum action);

  void add_mouse_action(int btn, bool hold, ActionEnum action);
  void add_mouse_action(int btn, int mod1, bool hold, ActionEnum action);
  void add_mouse_action(int btn, int mod1, int mod2, bool hold, ActionEnum action);

protected:
  void on_key_event(int key, int scancode, int action, int mods);
  void on_cursor_event(double xpos, double ypo);
  void on_mouse_btn_event(int button, int action, int mods);
  void handle_events();

  virtual void start_action(ActionEnum action) = 0;
  virtual void action_event(ActionEnum action) = 0;
  virtual void end_action(ActionEnum action) = 0;
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
    holding_keys.erase(key);
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
    mouse_hold.erase(btn);
  }
}

void Input::handle_events(){
  pressed_keys.clear();
  consumed_keys.clear();
  mouse_pressed.clear();
  consumed_mouse_btn.clear();
  activated_actions.clear();

  glfwPollEvents();

  for (Action act : key_actions){
    KeyData k = act.keys;
    ActionEnum action = act.action;

    std::unordered_map<int, bool>& key_map = k.hold ? holding_keys : pressed_keys;

    if (!consumed_keys[k.key1] && key_map[k.key1]
    && (k.key2 < 0 || holding_keys[k.key2])
    && (k.key3 < 0 || holding_keys[k.key3])){
      consumed_keys[k.key1] = true;

      if (k.hold){
        activated_actions[action] = true;
        if (!started_actions[action]){
          started_actions[action] = true;
          start_action(action);
        }
      }

      this->action_event(action);
    }
  }

  for (Action act : mouse_actions){
    KeyData k = act.keys;
    ActionEnum action = act.action;

    std::unordered_map<int, bool>& btn_map = k.hold ? mouse_hold : mouse_pressed;

    if (!consumed_mouse_btn[k.key1] && btn_map[k.key1]
    && (k.key2 < 0 || holding_keys[k.key2])
    && (k.key3 < 0 || holding_keys[k.key3])){
      // consumed_mouse_btn[k.key1] = true;

      if (k.hold){
        activated_actions[action] = true;
        if (!started_actions[action]){
          started_actions[action] = true;
          start_action(action);
        }
      }

      this->action_event(action);
    }
  }
  
  for (auto pair : started_actions){
    ActionEnum action = pair.first;
    if (!activated_actions[action]){
      started_actions[action] = false;
      end_action(action);
    }
  }
};

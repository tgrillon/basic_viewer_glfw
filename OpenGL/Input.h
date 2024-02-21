#include <GLFW/glfw3.h>
#include <unordered_map>
#include <queue>
#include <string>
#include <cctype>
#include <iomanip>

/**
 * TODO: Peux mieux faire, utiliser (int mod) pour shift/alt 
*/

struct KeyData {
  KeyData(){}
  KeyData(int key1, int key2, int key3, bool hold, bool mouse = false):
    key1(key1), key2(key2), key3(key3), hold(hold), mouse(mouse) {
      if (key1 >= 0) priority++;
      if (key2 >= 0) priority++;
      if (key3 >= 0) priority++;
    }

  int key1 = -1, key2 = -1, key3 = -1;
  bool hold = false;
  bool mouse = false;
  int priority = 0;
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

  std::unordered_map<ActionEnum, std::string> action_description;
  std::deque<Action> key_actions;
  Cursor cursor;

public:
  Cursor get_cursor() { return cursor; }

  static std::string get_key_string(KeyData keys);

  void add_action(int key, bool hold, ActionEnum action);
  void add_action(int key, int mod1, bool hold, ActionEnum action);
  void add_action(int key, int mod1, int mod2, bool hold, ActionEnum action);

  void add_mouse_action(int btn, bool hold, ActionEnum action);
  void add_mouse_action(int btn, int mod1, bool hold, ActionEnum action);
  void add_mouse_action(int btn, int mod1, int mod2, bool hold, ActionEnum action);

  void set_action_description(ActionEnum action, std::string description);
  std::string& get_action_description(ActionEnum action);
  std::unordered_map<ActionEnum, std::vector<KeyData>> get_action_keys();


protected:
  void on_key_event(int key, int scancode, int action, int mods);
  void on_cursor_event(double xpos, double ypo);
  void on_mouse_btn_event(int button, int action, int mods);
  void handle_events();

  virtual void start_action(ActionEnum action) = 0;
  virtual void action(ActionEnum action) = 0;
  virtual void end_action(ActionEnum action) = 0;
private:
  void add_action(KeyData keys, ActionEnum action);
};

std::string key_name(int key) {
  if (key >= 290 && key <= 314) // F1-25
    return "F" + char(key - 230);
  
  if (key >= 320 && key <= 329) // Keypad KP1-9
    return  "KP" + char(key - 260);

  switch (key) {
    case GLFW_KEY_UP: return "UP";
    case GLFW_KEY_DOWN: return "DOWN";
    case GLFW_KEY_LEFT: return "LEFT";
    case GLFW_KEY_RIGHT: return "RIGHT";
    case GLFW_KEY_RIGHT_SHIFT: return "RSHIFT";
    case GLFW_KEY_LEFT_SHIFT: return "LSHIFT";
    case GLFW_KEY_LEFT_CONTROL: return "LCTRL";
    case GLFW_KEY_RIGHT_CONTROL: return "RCTRL";
    case GLFW_KEY_LEFT_ALT: return "LALT";
    case GLFW_KEY_RIGHT_ALT: return "RALT";
  }

  std::string result = "???";

  const char* name = glfwGetKeyName(key, 0);

  if (name != nullptr){
    result = name;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
  }

  return result;
}

std::string Input::get_key_string(KeyData keys) {
  std::string result = "";

  // https://www.glfw.org/docs/3.3/group__keys.html
  
  // Assert keys.key1 > 0

  if (keys.key3 > 0)
    result += key_name(keys.key3) + "+";

  if (keys.key2 > 0)
    result += key_name(keys.key2) + "+";

  if (keys.mouse) {
    result += "MB";
    result += char(keys.key1 + 48);
    return result;
  }

  return result + key_name(keys.key1);

}

void Input::add_action(int key, int mod1, int mod2, bool hold, ActionEnum action) {
  add_action(KeyData(key, mod1, mod2, hold), action);
};

void Input::add_action(int key, int mod1, bool hold, ActionEnum action) {
  add_action(KeyData(key, mod1, -1, hold), action);
};

void Input::add_action(int key, bool hold, ActionEnum action) {
  add_action(KeyData(key, -1, -1, hold), action);
};

void Input::add_mouse_action(int btn, bool hold, ActionEnum action) {
  add_action(KeyData(btn, -1, -1, hold, true), action);
};

void Input::add_mouse_action(int btn, int mod1, bool hold, ActionEnum action) {
  add_action(KeyData(btn, mod1, -1, hold, true), action);
};

void Input::add_mouse_action(int btn, int mod1, int mod2, bool hold, ActionEnum action) {
  add_action(KeyData(btn, mod1, mod2, hold, true), action);
};

void Input::add_action(KeyData keys, ActionEnum action) {
  auto it = key_actions.begin();
  Action act(keys, action);

  while (it != key_actions.end()){
    if (it->keys.priority < keys.priority){
      key_actions.insert(it, act);
      return;
    }
    it++;
  }

  key_actions.push_back(act);
};

void Input::set_action_description(ActionEnum action, std::string description) {
  action_description[action] = description;
}

std::string& Input::get_action_description(ActionEnum action) {
  return action_description[action];
}

std::unordered_map<Input::ActionEnum, std::vector<KeyData>> Input::get_action_keys() {
  std::unordered_map<ActionEnum, std::vector<KeyData>> result;

  for (Action act : key_actions){
    if (result.count(act.action) == 0){
      result[act.action] = {act.keys};
      continue;
    }

    result[act.action].push_back(act.keys);
  }

  return result;
}


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
    
    std::unordered_map<int, bool>* key_map;

    if (k.mouse) 
      key_map = k.hold ? &mouse_hold : &mouse_pressed;
    else 
      key_map = k.hold ? &holding_keys : &pressed_keys;

    if (!consumed_keys[k.key1] && (*key_map)[k.key1]
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

      this->action(action);
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

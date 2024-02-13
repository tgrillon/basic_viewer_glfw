#include <GLFW/glfw3.h>
#include <unordered_map>

/**
 * TODO: Peux mieux faire, utiliser (int mod) pour shift/alt 
*/

struct KeyData {
  KeyData(int key1, int key2, int key3):
    key1(key1), key2(key2), key3(key3) {}

  int key1 = -1, key2 = -1, key3 = -1;

  bool operator==(const KeyData& other) const{
    return key1 == other.key1 && key2 == other.key2 && key3 == other.key3;
  }

};

template<>
struct std::hash<KeyData>{
  std::size_t operator()(const KeyData& k) const noexcept{ 
    std::size_t h1 = std::hash<int>{}(k.key1);
    std::size_t h2 = std::hash<int>{}(k.key2);
    std::size_t h3 = std::hash<int>{}(k.key3);
    return h1 ^ (h2 << 1) ^ (h3 << 1);
  }
};

class Input
{
public:
  using ActionEnum = int;
private:
  std::unordered_map<int, bool> holding_keys, pressed_keys;
  std::unordered_map<KeyData, int> pressed_actions, holding_actions;

public:
  void add_action(KeyData keys, bool hold, ActionEnum action);
  void add_action(int key1, bool hold, ActionEnum action);
  void add_action(int key1, int key2, bool hold, ActionEnum action);
  void add_action(int key1, int key2, int key3, bool hold, ActionEnum action);
protected:
  void on_key_event(int key, int scancode, int action, int mods);
  void handle_events();
  virtual void handle_actions(ActionEnum action) = 0;
};

void Input::add_action(int key1, int key2, int key3, bool hold, ActionEnum action) {
  add_action(KeyData(key1, key2, key3), hold, action);
};

void Input::add_action(int key1, int key2, bool hold, ActionEnum action) {
  add_action(KeyData(key1, key2, -1), hold, action);
};

void Input::add_action(int key1, bool hold, ActionEnum action) {
  add_action(KeyData(key1, -1, -1), hold, action);
};

void Input::add_action(KeyData keys, bool hold, ActionEnum action) {
  std::unordered_map<KeyData, int>& map = hold ? holding_actions : pressed_actions;
  map[keys] = action;
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

void Input::handle_events(){
  pressed_keys.clear();
  glfwPollEvents();

  for (auto pair : pressed_actions){
    KeyData k = pair.first;
    ActionEnum action = pair.second;

    if (pressed_keys[k.key1] 
    && (k.key2 < 0 || holding_keys[k.key2])
    && (k.key3 < 0 || holding_keys[k.key3])){
      handle_actions(action);
    }
  }

  for (auto pair : holding_actions){
    KeyData k = pair.first;
    ActionEnum action = pair.second;

    if (holding_keys[k.key1] 
    && (k.key2 < 0 || holding_keys[k.key2])
    && (k.key3 < 0 || holding_keys[k.key3])){
      handle_actions(action);
    }
  }
}


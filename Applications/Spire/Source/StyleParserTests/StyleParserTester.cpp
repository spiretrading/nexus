#include "Spire/StyleParserTests/StyleParserTester.hpp"
#include "Spire/StyleParser/DataTypes/SelectorParser.hpp"
#include "Spire/Styles/Selectors.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/DropDownBox.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/TableBody.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

void Spire::StyleParser::Tests::register_selection_types() {
  register_state_selector("disabled", Disabled());
  register_state_selector("hover", Hover());
  register_state_selector("focus", Focus());
  register_state_selector("focus_in", FocusIn());
  register_state_selector("checked", Checked());
  register_state_selector("press", Press());
  register_state_selector("pop_up", PopUp());
  register_state_selector("body", Body());
  register_state_selector("row", Row());
  register_type_selector("Button", is_a<Button>());
  register_type_selector("Icon", is_a<Icon>());
  register_type_selector("TextBox", is_a<TextBox>());
  register_pseudo_selector("placeholder", Placeholder());
}


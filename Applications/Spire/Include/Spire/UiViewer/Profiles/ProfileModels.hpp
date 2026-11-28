#ifndef SPIRE_UI_VIEWER_PROFILE_MODELS_HPP
#define SPIRE_UI_VIEWER_PROFILE_MODELS_HPP
#include "Nexus/Definitions/Scope.hpp"
#include "Spire/Spire/LocalQueryModel.hpp"
#include "Spire/Spire/TickerInfoToTickerQueryModel.hpp"
#include "Spire/Ui/KeyInputBox.hpp"

namespace Spire {

  /** Returns the sample tickers used in profiles. */
  std::shared_ptr<TickerInfoQueryModel> populate_ticker_query_model();

  /** Returns the sample tags used in profiles. */
  std::shared_ptr<LocalQueryModel<QString>> populate_tag_combo_box_model();

  /** Returns the sample scopes used in profiles. */
  std::shared_ptr<LocalQueryModel<Nexus::Scope>> populate_scope_box_model();

  /**
   * Checks a sample key sequence.
   * @param sequence The key sequence entered in the viewer.
   */
  QValidator::State key_input_box_validator(const QKeySequence& sequence);

  /**
   * Returns a validated key sequence model.
   * @param key The initial key sequence.
   */
  std::shared_ptr<KeySequenceValueModel>
    populate_key_input_box_model(const QKeySequence& key);
}

#endif

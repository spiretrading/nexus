#ifndef SPIRE_BLOTTERMODELUTILITIES_HPP
#define SPIRE_BLOTTERMODELUTILITIES_HPP
#include <functional>
#include <unordered_set>

namespace Spire {

  //! Performs an action on all models and their incoming links.
  /*!
    \param model The model to iterate over.
    \param f The action to perform on an individual model.
  */
  template<typename Model, typename F>
  void WithModels(const Model& model, F f) {
    std::unordered_set<const Model*> models;
    std::function<void (const Model&)> Populate =
      [&](const Model& model) {
        if(!models.insert(&model).second) {
          return;
        }
        f(model);
        const auto& incomingLinks = model.GetIncomingLinks();
        for(auto i = incomingLinks.begin(); i != incomingLinks.end(); ++i) {
          Populate(**i);
        }
      };
    Populate(model);
  }
}

#endif

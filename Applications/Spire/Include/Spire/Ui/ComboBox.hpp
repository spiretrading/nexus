#ifndef SPIRE_COMBO_BOX_HPP
#define SPIRE_COMBO_BOX_HPP
#include <any>
#include <QWidget>
#include <Beam/Queries/SnapshotLimit.hpp>
#include "Spire/Ui/Ui.hpp"
#include "Spire/Spire/QtPromise.hpp"

namespace Spire {

	class ComboBox : public QWidget {
		public:

			class QueryModel {
				public:

					struct Query {
						std::string m_text;
						Beam::Queries::SnapshotLimit m_limit;

						//! Returns an unlimited Query with empty text.
						static Query make_empty_query();
					};

					virtual ~QueryModel() = default;

					virtual QtPromise<std::vector<std::any>>
						query(const Query& query) const = 0;
			};

			explicit ComboBox(
				std::shared_ptr<QueryModel> model, QWidget* parent = nullptr);

		private:
			std::shared_ptr<QueryModel> m_model;
	};
}

#endif

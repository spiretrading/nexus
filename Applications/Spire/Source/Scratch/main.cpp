#include <QApplication>
#include <QTimer>
#include <memory>
#include <utility>
#include <cmath>
#include <Beam/Utilities/HashPtime.hpp>
#include <boost/functional/hash.hpp>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Styles/LinearExpression.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Spire;
using namespace Spire::Styles;

namespace Spire::Styles {

  template<typename T>
  class CubicBezierExpression {
    public:
      using Type = T;

      CubicBezierExpression(Expression<Type> start,
                            double x1, double y1,
                            double x2, double y2,
                            Expression<Type> end,
                            boost::posix_time::time_duration duration)
        : m_start(std::move(start)),
          m_x1(x1), m_y1(y1),
          m_x2(x2), m_y2(y2),
          m_end(std::move(end)),
          m_duration(duration) {}

      const Expression<Type>& get_start() const { return m_start; }
      double get_x1() const { return m_x1; }
      double get_y1() const { return m_y1; }
      double get_x2() const { return m_x2; }
      double get_y2() const { return m_y2; }
      const Expression<Type>& get_end() const { return m_end; }
      boost::posix_time::time_duration get_duration() const {
        return m_duration;
      }

      bool operator==(CubicBezierExpression const& o) const noexcept = default;

    private:
      Expression<Type> m_start, m_end;
      double m_x1, m_y1, m_x2, m_y2;
      boost::posix_time::time_duration m_duration;
  };
      static double ComputeBezier(double a1, double a2, double t) {
        double u = 1.0 - t;
        return 3.0*a1*u*u*t + 3.0*a2*u*t*t + t*t*t;
      }
      static double ComputeBezierDerivative(double a1, double a2, double t) {
        double u = 1.0 - t;
        return 3.0*u*u*a1 + 6.0*u*t*(a2 - a1) + 3.0*t*t*(1.0 - a2);
      }
      static double SolveForU(double x1, double x2, double t) {
        double u = t;
        for(int i = 0; i < 4; ++i) {
          double f  = ComputeBezier(x1, x2, u) - t;
          double df = ComputeBezierDerivative(x1, x2, u);
          if(std::abs(df) < 1e-6) break;
          u -= f/df;
          if(u <= 0.0) { u = 0.0; break; }
          if(u >= 1.0) { u = 1.0; break; }
        }
        return u;
      }

  template<typename S, typename U>
  CubicBezierExpression(S&&, double, double, double, double, U&&, boost::posix_time::time_duration)
    -> CubicBezierExpression<expression_type_t<S>>;

  // Evaluator specialization for our expression
  template<typename T>
  auto make_evaluator(CubicBezierExpression<T> expression,
                      const Stylist& stylist) {
    using Type = T;
    struct BEval {
      Evaluator<T> m_start, m_end;
      double m_x1, m_y1, m_x2, m_y2;
      boost::posix_time::time_duration m_duration;

      Evaluation<T> operator()(boost::posix_time::time_duration time) {
        using BS = boost::posix_time::time_duration;
        if(time <= seconds(0)) {
          auto v0 = m_start(seconds(0)).m_value;
          return Evaluation<T>(v0, seconds(0));
        }
        if(time >= m_duration) {
          auto v1 = m_end(m_duration).m_value;
          return Evaluation<T>(v1, boost::posix_time::pos_infin);
        }
        double t = double(time.total_milliseconds()) /
                   double(m_duration.total_milliseconds());
        double u = SolveForU(m_x1, m_x2, t);
        double y = ComputeBezier(m_y1, m_y2, u);
        auto s = m_start(time).m_value;
        auto e = m_end(time).m_value;
        T value = s + (e - s) * y;
        auto next_frame = [&] () -> boost::posix_time::time_duration {
          if(time >= m_duration) {
            return boost::posix_time::pos_infin;
          }
          return boost::posix_time::seconds(0);
        }();
        return Evaluation<T>(std::move(value), next_frame);
      }
    };
    return BEval{
      make_evaluator(expression.get_start(), stylist),
      make_evaluator(expression.get_end(),   stylist),
      expression.get_x1(), expression.get_y1(),
      expression.get_x2(), expression.get_y2(),
      expression.get_duration()
    };
  }

  /**
   * Convenience maker matching CSS syntax:
   *   cubic_bezier(start, x1, y1, x2, y2, end, duration)
   */
  template<typename S, typename U>
  auto cubic_bezier(S&& start,
                    double x1, double y1,
                    double x2, double y2,
                    U&& end,
                    boost::posix_time::time_duration duration) {
    return CubicBezierExpression(
      Expression<expression_type_t<S>>(std::forward<S>(start)),
      x1, y1, x2, y2,
      Expression<expression_type_t<S>>(std::forward<U>(end)),
      duration);
  }

  template<typename S, typename U>
  auto ease(S&& start, U&& end, time_duration duration) {
    return cubic_bezier(std::forward<S>(start), 0.25, 0.1, 0.25, 1,
      std::forward<U>(end), duration);
  }
} // namespace Spire::Styles

namespace std {
  template<typename T>
  struct hash<Spire::Styles::CubicBezierExpression<T>> {
    std::size_t operator ()(
        const Spire::Styles::CubicBezierExpression<T>& expression) const {
      auto seed = std::size_t(0);
      return seed;
    }
  };
}


struct ActivityMessageBox : QWidget {
  enum class State {
    NONE,
    DOWNLOADING,
    DOWNLOAD_COMPLETE,
    INSTALLING
  };
  TextBox* m_label;
  State m_state;

  ActivityMessageBox()
      : m_state(State::NONE) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto layout = make_vbox_layout(this);
    m_label = make_label(QString());
    layout->addWidget(m_label);
    m_label->get_current()->set(tr("Update available."));
    update_style(*m_label, [] (auto& style) {
      style.get(Any()).
        set(PaddingBottom(scale_height(6))).
        set(PaddingLeft(scale_width(76))).
        set(TextColor(QColor(0xFFFFFF)));
    });
  }

  void set_state(State state) {
    if(state == m_state) {
      return;
    }
    if(state == State::DOWNLOADING) {
      m_label->get_current()->set(tr("Downloading update..."));
      update_style(*m_label, [] (auto& style) {
        style.get(Any()).
          set(PaddingLeft(ease(scale_width(76), 0, milliseconds(800))));
      });
    }
  }
};

struct UpdaterBox : QWidget {
  ActivityMessageBox* m_activity_message_box;

  UpdaterBox() {
    auto body = new QWidget();
    auto layout = make_vbox_layout(body);
    layout->addSpacing(scale_height(38));
    m_activity_message_box = new ActivityMessageBox();
    layout->addWidget(m_activity_message_box);
    auto box = new Box(body);
    enclose(*this, *box);
    proxy_style(*this, *box);
    update_style(*this, [] (auto& style) {
      style.get(Any()).set(BackgroundColor(QColor(0x4B23A0)));
    });
    setFixedSize(scale(280, 232));
  }
};

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto window = UpdaterBox();
  window.show();
  QTimer::singleShot(1000, [&] {
    window.m_activity_message_box->set_state(
      ActivityMessageBox::State::DOWNLOADING);
  });
  application.exec();
}

#include "Nexus/Python/Tasks.hpp"
#include <Beam/Python/Tasks.hpp>
#include <Beam/Python/ToPythonTask.hpp>
#include "Nexus/Tasks/SingleOrderTask.hpp"
#include "Nexus/Tasks/SingleRedisplayableOrderTask.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionClient.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::ServiceLocator;
using namespace Beam::Tasks;
using namespace boost;
using namespace boost::python;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Python;
using namespace Nexus::Tasks;
using namespace std;

namespace {
  void SingleOrderTaskFactorySet(
      ToPythonTaskFactory<SingleOrderTaskFactory<VirtualOrderExecutionClient>>&
      factory, const OrderFields& fields) {
    factory.GetFactory().Set(fields);
  }
}

BEAM_DEFINE_PYTHON_POINTER_LINKER(
  ToPythonTask<SingleOrderTask<VirtualOrderExecutionClient>>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(
  ToPythonTaskFactory<SingleOrderTaskFactory<VirtualOrderExecutionClient>>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(
  ToPythonTask<SingleRedisplayableOrderTask>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(
  ToPythonTaskFactory<SingleRedisplayableOrderTaskFactory>);


void Nexus::Python::ExportSingleOrderTask() {
  using SingleOrderTask =
    Nexus::Tasks::SingleOrderTask<VirtualOrderExecutionClient>;
  using SingleOrderTaskFactory =
    Nexus::Tasks::SingleOrderTaskFactory<VirtualOrderExecutionClient>;
  class_<ToPythonTask<SingleOrderTask>,
    std::shared_ptr<ToPythonTask<SingleOrderTask>>, boost::noncopyable,
    bases<Task>>("SingleOrderTask", init<RefType<VirtualOrderExecutionClient>,
      std::shared_ptr<QueueWriter<const Order*>>, const OrderFields&>());
  implicitly_convertible<std::shared_ptr<ToPythonTask<SingleOrderTask>>,
    std::shared_ptr<Task>>();
  {
    class_<ToPythonTaskFactory<SingleOrderTaskFactory>,
      bases<VirtualTaskFactory>>("SingleOrderTaskFactory", init<
      RefType<VirtualOrderExecutionClient>,
      std::shared_ptr<QueueWriter<const Order*>>, const DirectoryEntry&>())
      .def(init<RefType<VirtualOrderExecutionClient>,
        std::shared_ptr<QueueWriter<const Order*>>, const OrderFields&>())
      .def("__copy__", &MakeCopy<ToPythonTaskFactory<SingleOrderTaskFactory>>)
      .def("__deepcopy__", &MakeDeepCopy<
      ToPythonTaskFactory<SingleOrderTaskFactory>>)
      .add_static_property("SECURITY", make_getter(
        &BaseSingleOrderTaskFactory::SECURITY))
      .add_static_property("ORDER_TYPE", make_getter(
        &BaseSingleOrderTaskFactory::ORDER_TYPE))
      .add_static_property("SIDE", make_getter(
        &BaseSingleOrderTaskFactory::SIDE))
      .add_static_property("DESTINATION", make_getter(
        &BaseSingleOrderTaskFactory::DESTINATION))
      .add_static_property("QUANTITY", make_getter(
        &BaseSingleOrderTaskFactory::QUANTITY))
      .add_static_property("PRICE", make_getter(
        &BaseSingleOrderTaskFactory::PRICE))
      .add_static_property("CURRENCY", make_getter(
        &BaseSingleOrderTaskFactory::CURRENCY))
      .add_static_property("TIME_IN_FORCE", make_getter(
        &BaseSingleOrderTaskFactory::TIME_IN_FORCE))
      .def("set", &SingleOrderTaskFactorySet);
    implicitly_convertible<ToPythonTaskFactory<SingleOrderTaskFactory>,
      TaskFactory>();
  }
  ExportTaskFactoryProperty<CurrencyId>();
  ExportTaskFactoryProperty<Destination>();
  ExportTaskFactoryProperty<Money>();
  ExportTaskFactoryProperty<OrderType>();
  ExportTaskFactoryProperty<Quantity>();
  ExportTaskFactoryProperty<Security>();
  ExportTaskFactoryProperty<Side>();
  ExportTaskFactoryProperty<TimeInForce>();
}

void Nexus::Python::ExportSingleRedisplayableOrderTask() {
  class_<ToPythonTask<SingleRedisplayableOrderTask>,
    std::shared_ptr<ToPythonTask<SingleRedisplayableOrderTask>>,
    boost::noncopyable, bases<Task>>("SingleRedisplayableOrderTask",
    init<const TaskFactory&, Quantity>());
  implicitly_convertible<std::shared_ptr<
    ToPythonTask<SingleRedisplayableOrderTask>>, std::shared_ptr<Task>>();
  {
    class_<ToPythonTaskFactory<SingleRedisplayableOrderTaskFactory>,
      bases<VirtualTaskFactory>>("SingleRedisplayableOrderTaskFactory", init<
      const TaskFactory&>())
      .def("__copy__", &MakeCopy<
        ToPythonTaskFactory<SingleRedisplayableOrderTaskFactory>>)
      .def("__deepcopy__", &MakeDeepCopy<
        ToPythonTaskFactory<SingleRedisplayableOrderTaskFactory>>)
      .add_static_property("QUANTITY", make_getter(
        &SingleRedisplayableOrderTaskFactory::QUANTITY))
      .add_static_property("DISPLAY", make_getter(
        &SingleRedisplayableOrderTaskFactory::DISPLAY));
    implicitly_convertible<
      ToPythonTaskFactory<SingleRedisplayableOrderTaskFactory>, TaskFactory>();
  }
}

void Nexus::Python::ExportTasks() {
  string nestedName = extract<string>(scope().attr("__name__") + ".tasks");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("tasks") = nestedModule;
  scope parent = nestedModule;
  ExportSingleOrderTask();
  ExportSingleRedisplayableOrderTask();
}

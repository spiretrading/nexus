import View from './view';

class Controller {
  constructor(react) {
    this.componentModel = {
      id: react.props.id,
      width: react.props.width,
      height: react.props.height,
    };
    this.view = new View(react, this, this.componentModel);
  }

  /** @private */
  onShow(eventName, preloaderId){
    if (preloaderId === this.componentModel.id) {
      this.view.show();
    }
  }

  /** @private */
  onHide(eventName, preloaderId){
    if (preloaderId === this.componentModel.id) {
      this.view.hide();
    }
  }

  notifyPreloaderHidden() {
    EventBus.publish(Event.Application.PRELOADER_HIDDEN, this.componentModel.id);
  }

  getView() {
    return this.view;
  }

  componentDidMount() {
    this.showEventListenerId = EventBus.subscribe(Event.Application.SHOW_PRELOADER, this.onShow.bind(this));
    this.hideEventListenerId = EventBus.subscribe(Event.Application.HIDE_PRELOADER, this.onHide.bind(this));
  }

  componentWillUnmount() {
    EventBus.unsubscribe(this.showEventListenerId);
    EventBus.unsubscribe(this.hideEventListenerId);
  }
}

export default Controller;

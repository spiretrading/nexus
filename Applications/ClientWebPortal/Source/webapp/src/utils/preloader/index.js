import React from 'react';
import ReactDOM from 'react-dom';
import Preloader from 'components/reusables/preloader';

class PreloaderUtils {
  show(parentElement, id, width, height) {
    let style = "position: absolute; top: 0; left: 0; width: 100%; height: 100%; background: rgba(255, 255, 255, 0.5);";
    let containerId = id + '-preloader-container';
    let $container = $('<div id="' + containerId + '" style="' + style + '"></div>');
    $(parentElement).append($container);
    ReactDOM.render(<Preloader id={id} width={width} height={height}/>, $container[0]);
  }

  showOnEntireScreen(id, width, height) {
    let style = "position: fixed; top: 0; left: 0; width: 100vw; height: 100vh; background: rgba(255, 255, 255, 0.5);";
    let containerId = id + '-preloader-container';
    let $container = $('<div id="' + containerId + '" style="' + style + '"></div>');
    $('body').append($container);
    ReactDOM.render(<Preloader id={id} width={width} height={height}/>, $container[0]);
    EventBus.publish(Event.Application.SHOW_PRELOADER, id);
  }

  hide(id) {
    let containerId = id + '-preloader-container';
    let $container = $('#' + containerId);
    let resolve;
    let promise = new Promise((aResolve, aReject) => {
      resolve = aResolve;
    });

    EventBus.publish(Event.Application.HIDE_PRELOADER, id);
    let eventListenerId = EventBus.subscribe(Event.Application.PRELOADER_HIDDEN, (eventName, preloaderId) => {
      EventBus.unsubscribe(eventListenerId);
      if (preloaderId === id) {
        ReactDOM.unmountComponentAtNode($container[0]);
        $container.remove();
        resolve();
      }
    });

    return promise;
  }
}

export default new PreloaderUtils();
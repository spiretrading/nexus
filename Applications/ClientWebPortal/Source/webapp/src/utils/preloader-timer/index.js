import preloader from 'utils/preloader';
import uuid from 'uuid';

/** Preloader timer displays the preloader only when asynchronous results were not received in time */
class PreloaderTimer {
  start(taskPromise, targetElement, width, height) {
    let isTimedOut = false;
    let preloaderId;
    let timeout = setTimeout(() => {
      isTimedOut = true;
      preloaderId = uuid.v4();
      if (targetElement != null) {
        preloader.show(targetElement, preloaderId, width, height);
      } else {
        preloader.showOnEntireScreen(preloaderId, width, height);
      }
    }, Config.PRELOADER_SHOW_TIME);

    let resolve;
    let promise = new Promise((aResolve, aReject) => {
      resolve = aResolve;
    });

    taskPromise.then((parameter) => {
      if (isTimedOut) {
        preloader.hide(preloaderId).then(() => {
          resolve(parameter);
        });
      } else {
        clearTimeout(timeout);
        resolve(parameter);
      }
    });

    return promise;
  }
}

export default new PreloaderTimer();

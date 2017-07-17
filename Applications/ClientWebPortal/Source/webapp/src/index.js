import ES6Promise from 'es6-promise';
import {render} from 'react-dom';
import React from 'react';
import {Router, browserHistory} from 'react-router';
import rootRoute from 'routes/';
import 'jquery-ui';
import 'jquery-ui-css';
import 'jquery-tagit';
import 'jquery-tagit-css';
import 'jquery-slimscroll';
import 'jquery-ellipsis';
import 'jquery-ellipsis-css';
import 'index.html';
import 'bootstrap-css';
import 'styles/general.scss';
import configs from './configs';
import eventBus from 'utils/event-bus';
import event from 'utils/event-bus/events';
import userService from 'services/user';
import {ServiceLocatorClient, DirectoryEntry} from 'spire-client';
import sessionInitializer from 'commons/session-initializer';

window.clone = (originalObj) => {
  if (originalObj != null) {
    if (originalObj.constructor == Array) {
      let cloneArray = [];
      for (let i=0; i<originalObj.length; i++) {
        cloneArray.push(window.clone(originalObj[i]));
      }
      return cloneArray;
    } else if (typeof originalObj == 'object') {
      if (originalObj.clone != null) {
        return originalObj.clone.call(originalObj);
      } else {
        let cloneObj = {};
        for (let property in originalObj) {
          if (originalObj[property] != null) {
            cloneObj[property] = clone(originalObj[property]);
          }
        }
        return cloneObj;
      }
    } else {
      return JSON.parse(JSON.stringify(originalObj));
    }
  } else {
    return null;
  }
};
window.overwriteMerge = (originalObj, newObj) => {
  for (let property in originalObj) {
    if (newObj[property] !== undefined){
      originalObj[property] = newObj[property];
    }
  }
  $.extend(true, originalObj, newObj);
};
window.EventBus = eventBus;
window.Event = event;
window.Config = configs;
ES6Promise.polyfill();

let serviceLocatorClient = new ServiceLocatorClient();

serviceLocatorClient.loadCurrentAccount.call(serviceLocatorClient)
  .then((account) => {
    let accountDirectoryEntry = new DirectoryEntry(
      account.id,
      account.type,
      account.name
    );
    if (!isLoggedIn(accountDirectoryEntry)) {
      if (window.location.href != Config.HOME_URL) {
        window.location.href = Config.HOME_URL;
      } else {
        renderGUI();
      }
    } else {
      userService.initialize(accountDirectoryEntry)
        .then(sessionInitializer.initialize.bind(sessionInitializer))
        .then(renderGUI)
        .then(publishSignedInEvent)
    }

    function isLoggedIn(accountDirectoryEntry) {
      return accountDirectoryEntry.type != -1;
    }

    function renderGUI() {
      render(
        <Router history={browserHistory} routes={rootRoute}/>,
        document.getElementById('root')
      );
    }

    function publishSignedInEvent() {
      let appRenderedEventListenerId;
      appRenderedEventListenerId = EventBus.subscribe(Event.Application.RENDERED, () => {
        EventBus.unsubscribe(appRenderedEventListenerId);
        EventBus.publish(Event.Application.SIGNED_IN);
      });
    }
  });

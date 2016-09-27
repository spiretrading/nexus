import definitionsService from 'services/definitions';

/** When user successfully logs in, there are certain data to be loaded to initialize the session */
class SessionInitializer {
  initialize() {
    return Promise.all([
      definitionsService.initialize.apply(definitionsService)
    ]);
  }
}

export default new SessionInitializer();

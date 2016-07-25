import httpConnectionManager from '../commons/http-connection-manager';
import ResultCodes from './result-codes.js';
import accountRoles from '../commons/account-roles';
const ResultCode = ResultCodes;

/** Spire admin client class */
class AdminClient {
  /** @private */
  logErrorAndThrow(xhr) {
    console.error('Unexpected error happened.');
    throw ResultCode.ERROR;
  }

  loadAccountRoles(directoryEntry) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'administration_service/load_account_roles';
    let payload = {
      account: directoryEntry
    };

    return httpConnectionManager.send(apiPath, payload)
      .then(onResponse)
      .catch(this.logErrorAndThrow);

    function onResponse(roles) {
      return accountRoles.parse(roles);
    }
  }

  loadAccountProfile(directoryEntry) {
    let apiPath = Config.BACKEND_API_ROOT_URL + 'administration_service/load_account_profile';
    let payload = directoryEntry;

    // return httpConnectionManager.send(apiPath, payload)
    //   .then((response) => {
    //     console.debug('response received');
    //     console.debug(response);
    //     throw "temp error";
    //   })
    //   .catch(this.logErrorAndThrow);

    // TODO: temporarily mocked
    return new Promise(function(resolve, reject) {
      let response = {
        pictureUrl: 'http://www.globemoon.net/waftf/amasia/akico-pic.jpg',
        name: 'Trista Pokemon',
        userName: 'tPokemon123',
        registrationDate: '2016-07-21T20:53:34Z',
        accountNumber: '0812301',
        email: 'trista@pokemon.com',
        address: '666 Pikachu Vill',
        city: 'Toronto',
        province: 'Ontario',
        postalCode: 'M5B 1P3',
        country: 'Canada',
        lastSignin: '2016-07-21T20:53:34Z'
      };

      setTimeout(() => {
        resolve(response);
      }, getRandomInt(0, 1500));

      function getRandomInt(min, max) {
        return Math.floor(Math.random() * (max - min + 1)) + min;
      }
    });
  }
}

export default new AdminClient();
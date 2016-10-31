import {ServiceLocatorResultCode} from 'spire-client';

let clientCodes = [
  ServiceLocatorResultCode
];

let codes = {
  SUCCESS: 1000,
  FAIL: 1001,
  ERROR: 1002
};

for (let i=0; i<clientCodes; i++) {
  for (let property in clientCodes[i]) {
    if (property != 'SUCCESS' &&
      property != 'FAIL' &&
      property != 'ERROR') {
      codes[property] = clientCodes[i][property];
    }
  }
}

export default codes;

import {post, plugins} from 'popsicle';

/** Spire HTTP connection manager */
class HTTPConnectionManager {
  send(apiPath, payload, isJsonResponse) {
    let options = {
      url: apiPath
    };

    if (payload != null) {
      options.headers = {
        'Content-Type': 'application/json'
      };
      options.body = JSON.stringify(payload);
    }

    return post(options)
      .then((response) => {
        if (response.status == 200) {
          if (isJsonResponse) {
            return JSON.parse(response.body);
          } else {
            return null;
          }
        } else {
          throw new Error(response.status);
        }
      });
  }
}

export default new HTTPConnectionManager();

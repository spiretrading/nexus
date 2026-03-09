import * as Nexus from 'nexus';
import * as React from 'react';
import { RequestsModel } from './requests_model';

interface Properties {

  /** The account's roles. */
  roles: Nexus.AccountRoles;

  /** The model to use. */
  model: RequestsModel;
}

/** Implements a controller for the RequestsPage. */
export class RequestsController extends React.Component<Properties> {
  public render(): JSX.Element {
    return <div/>;
  }
}

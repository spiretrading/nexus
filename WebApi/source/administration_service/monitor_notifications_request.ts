import * as Beam from 'beam';
import { Notification } from './notification';

/** Request to monitor notifications for an account. */
export class MonitorNotificationsRequest
    implements Beam.ServiceRequest<Notification.Id> {

  public readonly service =
    'Nexus.AdministrationServices.MonitorNotificationsService';

  /**
   * Constructs a MonitorNotificationsRequest.
   * @param account - The account to monitor.
   */
  constructor(private readonly _account: Beam.DirectoryEntry) {}

  public toJson(): any {
    return {
      account: this._account.toJson()
    };
  }

  public parseResponse(value: any): Notification.Id {
    return value;
  }
}

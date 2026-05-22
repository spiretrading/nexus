import * as Beam from 'beam';
import { Notification } from './notification';

/** A message indicating a new notification for an account. */
export class NotificationMessage extends Beam.Message {
  public static readonly TYPE =
    'Nexus.AdministrationService.NotificationMessage';

  public static fromJson(data: any): NotificationMessage {
    return new NotificationMessage(Notification.fromJson(data.notification));
  }

  /**
   * Constructs a NotificationMessage.
   * @param notification - The new notification.
   */
  constructor(public readonly notification: Notification) {
    super(NotificationMessage.TYPE, {
      notification: notification.toJson()
    });
  }
}

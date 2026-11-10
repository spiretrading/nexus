import * as assert from 'node:assert/strict';
import { describe, it } from 'node:test';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { getNotificationUrl } from
  '../../source/pages/notifications_page/notification_url';

describe('NotificationUrl', () => {
  it('opens_completed_and_failed_reports', () => {
    for(const status of ['COMPLETED', 'FAILED']) {
      const notification = new Nexus.Notification('notification',
        Beam.DirectoryEntry.INVALID, '',
        JSON.stringify({report_id: 'report /?#', status}),
        Nexus.Notification.Category.REPORT);
      assert.equal(getNotificationUrl(notification),
        '/reports/report%20%2F%3F%23');
    }
  });

  it('falls_back_for_invalid_report_data', () => {
    for(const data of ['', '{', 'null', '[]', '{}', '{"report_id":null}',
        '{"report_id":42}', '{"report_id":""}']) {
      const notification = new Nexus.Notification('notification',
        Beam.DirectoryEntry.INVALID, '', data,
        Nexus.Notification.Category.REPORT);
      assert.equal(getNotificationUrl(notification),
        '/notifications/notification');
    }
  });

  it('preserves_account_modification_links', () => {
    const notification = new Nexus.Notification('notification',
      Beam.DirectoryEntry.INVALID, '', '{"request_id":123}',
      Nexus.Notification.Category.ACCOUNT_MODIFICATION);
    assert.equal(getNotificationUrl(notification), '/request_history/123');
  });
});

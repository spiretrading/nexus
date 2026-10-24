import * as Beam from 'beam';
import { AccountGroupQueryModel } from
  '../../components/edit_account_group_modal';

/** Restricts report sharing to individual accounts and groups. */
export class ReportRecipientQueryModel extends AccountGroupQueryModel {

  /** The underlying account and group lookup model. */
  public readonly source: AccountGroupQueryModel;

  /** Constructs a recipient lookup over an existing account lookup. */
  constructor(source: AccountGroupQueryModel) {
    super();
    this.source = source;
  }

  public async parse(query: string): Promise<Beam.DirectoryEntry> {
    const entry = await this.source.parse(query);
    if(isReportRecipient(entry)) {
      return entry;
    }
    return null;
  }

  public async submit(query: string): Promise<readonly Beam.DirectoryEntry[]> {
    return (await this.source.submit(query)).filter(isReportRecipient);
  }
}

/** Returns whether an entry can receive a shared report. */
export function isReportRecipient(entry: Beam.DirectoryEntry): boolean {
  return entry != null && (entry.type === Beam.DirectoryEntry.Type.ACCOUNT ||
    entry.type === Beam.DirectoryEntry.Type.DIRECTORY) &&
      !entry.equals(Beam.DirectoryEntry.STAR_DIRECTORY);
}

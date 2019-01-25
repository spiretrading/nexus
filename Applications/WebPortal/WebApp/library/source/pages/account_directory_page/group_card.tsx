import * as Beam from 'beam';
import * as React from 'react';
import { DisplaySize } from '../../display_size';
import { AccountEntry } from '.';

interface Properties {

  /** Determines the size to display the component at. */
  displaySize: DisplaySize;

  /** The group the card belongs to. */
  group: Beam.DirectoryEntry;

  /** The accounts in the group. */
  accounts: AccountEntry[];

  /** The current filter used on the accounts. */
  filter: string;

  /**  Determines if the card is opened. */
  isOpen: boolean;

  /** Called when the card is opened. */
  onClick: (group: Beam.DirectoryEntry) => void;
}

/** A card that displays a group and the accounts associated with it. */
export class GroupCard extends React.Component<Properties> {
  public render(): JSX.Element {
    return <div style={GroupCard.STYLE.test}>Group</div>;
  }

  private static readonly STYLE = {
    test: {
      boxSizing: 'border-box' as 'border-box',
      width: '100%',
      height: '40px',
      backgroundColor: '#F8F8F8',
      border: '1px solid #684BC7'
    }
  };
}

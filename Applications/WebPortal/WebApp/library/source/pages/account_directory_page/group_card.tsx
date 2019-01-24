import * as Beam from 'beam';
import * as React from 'react';
import { DisplaySize } from '../../display_size';
import { AccountEntry } from '.';

interface Properties {
  displaySize: DisplaySize;
  group: Beam.DirectoryEntry;
  accounts: AccountEntry[];
  filter: string;
}

export class GroupCard extends React.Component<Properties> {
  public render(): JSX.Element {
    return <div style={GroupCard.STYLE.test}/>;
  }

  private static readonly STYLE = {
    test: {
      width: '100%',
      height: '40px',
      backgroundColor: '#F8F8F8'
      }
    };
}

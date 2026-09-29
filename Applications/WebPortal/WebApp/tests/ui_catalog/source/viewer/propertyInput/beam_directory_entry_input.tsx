import * as Beam from 'beam';
import * as React from 'react';

interface Properties {
  value: Beam.DirectoryEntry;
  update: (value: Beam.DirectoryEntry) => void;
}

/** Edits the type, ID and name of an account or directory entry. */
export class BeamDirectoryEntryInput extends React.Component<Properties> {
  public render(): JSX.Element {
    return (
      <div style={{display: 'flex', flexWrap: 'wrap', gap: '4px'}}>
        <select aria-label='Entry type' value={this.props.value.type}
            onChange={this.onTypeChange}>
          <option value={Beam.DirectoryEntry.Type.ACCOUNT}>ACCOUNT</option>
          <option value={Beam.DirectoryEntry.Type.DIRECTORY}>DIRECTORY</option>
        </select>
        <input type='number' aria-label='Entry ID' style={{width: '60px'}}
          value={this.props.value.id} onChange={this.onIdChange}/>
        <input aria-label='Entry name' value={this.props.value.name}
          onChange={this.onNameChange}/>
      </div>);
  }

  private onTypeChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    this.props.update(new Beam.DirectoryEntry(Number(event.target.value),
      this.props.value.id, this.props.value.name));
  };

  private onIdChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.props.update(new Beam.DirectoryEntry(this.props.value.type,
      event.target.valueAsNumber, this.props.value.name));
  };

  private onNameChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.props.update(new Beam.DirectoryEntry(this.props.value.type,
      this.props.value.id, event.target.value));
  };
}

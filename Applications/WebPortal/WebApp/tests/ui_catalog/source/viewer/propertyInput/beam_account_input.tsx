import * as Beam from 'beam';
import * as React from 'react';

interface Properties {

  /** The value of the field. */
  value?: Beam.DirectoryEntry;

  /** The callback to update the value. */
  update?: (newValue: Beam.DirectoryEntry) => void;
}

/** An input for specifying a Beam.DirectoryEntry account (id and username). */
export class BeamAccountInput extends React.Component<Properties> {
  public render(): JSX.Element {
    return (
      <div style={{display: 'flex', gap: '4px'}}>
        <input type='number' style={{width: '60px'}}
          value={this.props.value.id} onChange={this.onIdChange}
          onWheel={this.onWheel}/>
        <input value={this.props.value.name} onChange={this.onNameChange}/>
      </div>);
  }

  private onIdChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.props.update(Beam.DirectoryEntry.makeAccount(
      event.target.valueAsNumber, this.props.value.name));
  }

  private onNameChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.props.update(Beam.DirectoryEntry.makeAccount(
      this.props.value.id, event.target.value));
  }

  private onWheel = (event: React.WheelEvent<HTMLInputElement>) => {
    if(document.activeElement !== event.currentTarget) {
      return;
    }
    event.preventDefault();
    const delta = event.deltaY < 0 ? 1 : -1;
    this.props.update(Beam.DirectoryEntry.makeAccount(
      this.props.value.id + delta, this.props.value.name));
  }
}

import * as React from 'react';
import { DisplaySize } from '../../..';
import { FormEntry } from './form_entry';
import { TextField } from './text_field';

interface Properties {

  /** Determines the layout used to display the page. */
  displaySize: DisplaySize;
}

/** Displays an account's profile page. */
export class ProfilePage extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.onInput = this.onInput.bind(this);
  }

  public render(): JSX.Element {
    const orientation = ( () => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return FormEntry.Orientation.VERTICAL;
      } else {
        return FormEntry.Orientation.HORIZONTAL;
      }
    })();
    return (<div>
      <FormEntry name='test'
      orientation={orientation}
      children={<TextField displaySize={this.props.displaySize}/>}/>
      <FormEntry name='test'
      orientation={orientation}
      children={ <TextField displaySize={this.props.displaySize}
      value={'zbeep'}/>}/>
    </div>);
  }

  private onInput(value: string) {
    console.log(value);
    this.setState({
      comment: value
    });
  }

}

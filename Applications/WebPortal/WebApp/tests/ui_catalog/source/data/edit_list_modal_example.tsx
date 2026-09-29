import * as React from 'react';
import { Button, EditListModal } from 'web_portal';

interface Properties {
  title: string;
  titleSingle: string;
  listHeading: string;
  listHeadingSingle: string;
  items: string[];
  selected: string[];
  selectionMode: EditListModal.SelectionMode;
  readOnly: boolean;
  onSubmit: (selected: string[]) => void;
  onClose: () => void;
}

interface State {
  isOpen: boolean;
}

/** A catalog example that opens and closes the list editor. */
export class EditListModalExample extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {isOpen: false};
  }

  public render(): JSX.Element {
    return (
      <div>
        <Button label='Open editor' onClick={this.onOpen}/>
        <p>Selection: {this.props.selected.join(', ') || '(empty)'}</p>
        {this.state.isOpen &&
          <EditListModal {...this.props} getLabel={this.getLabel}
            onSubmit={this.onSubmit} onClose={this.onClose}/>}
      </div>);
  }

  private getLabel = (item: string) => item;

  private onOpen = () => {
    this.setState({isOpen: true});
  };

  private onClose = () => {
    this.setState({isOpen: false});
    this.props.onClose();
  };

  private onSubmit = (selected: string[]) => {
    this.props.onSubmit(selected);
    this.setState({isOpen: false});
  };
}

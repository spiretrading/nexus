import * as React from 'react';
import { ListInput, LocalQueryModel } from 'web_portal';

interface Properties {
  value: string[];
  items: string[];
  title: string;
  listHeading: string;
  placeholder: string;
  readOnly: boolean;
  disabled: boolean;
  onChange: (value: string[]) => void;
}

/** Demonstrates a list input backed by a local query model. */
export class ListInputExample extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.items = null;
    this.model = null;
  }

  public render(): JSX.Element {
    const {items, ...props} = this.props;
    return <ListInput {...props} model={this.getModel()} getLabel={String}
      aria-label='Selected items'/>;
  }

  private getModel(): LocalQueryModel<string> {
    if(this.items !== this.props.items) {
      this.items = this.props.items;
      this.model = new LocalQueryModel(String);
      for(const item of this.items) {
        this.model.add(item);
      }
    }
    return this.model;
  }

  private items: readonly string[];
  private model: LocalQueryModel<string>;
}

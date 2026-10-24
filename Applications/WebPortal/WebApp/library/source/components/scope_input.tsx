import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../display_size';
import { combineScopes, getScopeLabel, QueryModel, splitScope } from
  '../models';
import { EditScopeModal } from './edit_scope_modal';
import { ListInput } from './list_input';

interface Properties extends
    Omit<React.InputHTMLAttributes<HTMLInputElement>,
      'value' | 'defaultValue' | 'onChange'> {

  /** The scope displayed in the input. Defaults to an empty scope. */
  value?: Nexus.Scope;

  /** The model used to parse scope entries and load suggestions. */
  model: QueryModel<Nexus.Scope>;

  /** Retained for existing callers. Layout follows the viewport size. */
  displaySize?: DisplaySize;

  /** Called when the user submits a new scope. */
  onChange?: (value: Nexus.Scope) => void;
}

/** Displays a scope with a modal editor. */
export class ScopeInput extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.value = props.value;
    this.entries = splitScope(props.value ?? new Nexus.Scope());
  }

  public render(): JSX.Element {
    const {value, model, displaySize, onChange, ...rest} = this.props;
    if(this.value !== value) {
      this.value = value;
      this.entries = splitScope(value ?? new Nexus.Scope());
    }
    return <ListInput {...rest} value={this.entries} model={model}
      getLabel={getScopeLabel} title='Edit Scope' listHeading='Added Scope'
      placeholder={this.props.placeholder ??
        'Enter countries, venues, or tickers'}
      editListModal={EditScopeModal} onChange={this.onChange}/>;
  }

  private onChange = (entries: Nexus.Scope[]) => {
    this.props.onChange?.(combineScopes(entries, this.props.value?.name ?? ''));
  };

  private value: Nexus.Scope;
  private entries: Nexus.Scope[];
}

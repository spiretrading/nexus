import * as Nexus from 'nexus';
import * as React from 'react';
import { combineScopes, getScopeLabel, LocalTickerQueryModel, ScopeQueryModel }
  from 'web_portal';

interface Properties {
  value: Nexus.Scope;
  update: (value: Nexus.Scope) => void;
}

interface State {
  text: string;
  error: string;
}

/** Edits a scope property as comma-separated entries. */
export class ScopeValueInput extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {text: getScopeLabel(props.value), error: ''};
    this.model = new ScopeQueryModel(new LocalTickerQueryModel([]));
    this.version = 0;
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.value !== this.props.value) {
      ++this.version;
      this.setState({text: getScopeLabel(this.props.value), error: ''});
    }
  }

  public componentWillUnmount(): void {
    ++this.version;
  }

  public render(): JSX.Element {
    return <div>
      <input aria-label='Scope entries' value={this.state.text}
        onChange={this.onChange}/>
      <button onClick={this.onApply}>Apply scope</button>
      {this.state.error && <span role='alert'>{this.state.error}</span>}
    </div>;
  }

  private onChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    ++this.version;
    this.setState({text: event.target.value, error: ''});
  };

  private onApply = async () => {
    const version = ++this.version;
    const text = this.state.text.split(',').map(entry => entry.trim()).
      filter(entry => entry !== '');
    const entries = await Promise.all(
      text.map(entry => this.model.parse(entry)));
    if(version !== this.version) {
      return;
    }
    if(entries.some(entry => entry === null)) {
      this.setState({error: 'Enter a country, venue, qualified ticker, or *.'});
      return;
    }
    this.props.update(combineScopes(entries, this.props.value.name));
  };

  private model: ScopeQueryModel;
  private version: number;
}

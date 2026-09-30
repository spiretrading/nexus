import { css, StyleSheet } from 'aphrodite/no-important';
import * as Nexus from 'nexus';
import * as React from 'react';
import { Button, TickerInput } from '../../../components';
import { QueryModel } from '../../../models';

interface Properties {

  /** The ticker-valued compliance parameters. */
  value: readonly Nexus.Ticker[];

  /** The model used to parse tickers and load suggestions. */
  model: QueryModel<Nexus.Ticker>;

  /** Whether entries can only be viewed. */
  readOnly: boolean;

  /** Called when a ticker is edited, added, or removed. */
  onChange: (value: Nexus.Ticker[]) => void;
}

/** Edits a ticker-list parameter using individual ticker fields. */
export class TickerParameterList extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.nextIdentifier = 0;
    this.rows = [];
  }

  public render(): JSX.Element {
    const remaining = this.rows.slice();
    this.rows = this.props.value.map(value => {
      const index = remaining.findIndex(row => row.value === value);
      if(index !== -1) {
        return remaining.splice(index, 1)[0];
      }
      return {id: this.nextIdentifier++, value};
    });
    return <div className={css(STYLES.list)}>
      {this.rows.map(({id, value}, index) =>
        <div key={id} className={css(STYLES.row)}>
          <div className={css(STYLES.field)}>
            <TickerInput value={value} model={this.props.model}
              required
              readOnly={this.props.readOnly}
              aria-label={`Ticker ${index + 1}`}
              onChange={value => this.onChange(index, value)}/>
          </div>
          {!this.props.readOnly &&
            <button type='button' aria-label={`Remove ticker ${index + 1}`}
                className={css(STYLES.remove)}
                onClick={() => this.onRemove(index)}>
              Remove
            </button>}
        </div>)}
      {!this.props.readOnly &&
        <Button label='Add ticker' onClick={this.onAdd}/>}
    </div>;
  }

  private onChange = (index: number, value: Nexus.Ticker) => {
    const tickers = this.props.value.slice();
    tickers[index] = value;
    this.rows[index].value = value;
    this.props.onChange(tickers);
  };

  private onAdd = () => {
    this.props.onChange([...this.props.value, Nexus.Ticker.NONE]);
  };

  private onRemove = (index: number) => {
    this.rows.splice(index, 1);
    this.props.onChange(this.props.value.filter((_, i) => i !== index));
  };

  private nextIdentifier: number;
  private rows: {id: number; value: Nexus.Ticker}[];
}

const STYLES = StyleSheet.create({
  list: {display: 'flex', flexDirection: 'column', gap: '8px'},
  row: {display: 'flex', alignItems: 'center', gap: '8px'},
  field: {flex: 1, minWidth: 0},
  remove: {
    border: 0,
    padding: 0,
    background: 'transparent',
    font: 'inherit',
    color: '#684BC7',
    cursor: 'pointer'
  }
});

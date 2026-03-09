import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { DiffBadge } from './diff_badge';
import { RequestsModel } from './requests_model';

interface Properties {

  /** The name of the property to change. */
  name: string;

  /** The old value of the property. */
  oldValue: string;

  /** The new value of the property. */
  newValue: string;

  /** The Delta from old_value to new_value. */
  delta: RequestsModel.Delta;
}

interface State {
  isWide: boolean;
}

/** Displays a risk controls change item. */
export class RiskControlsChangeItem extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.containerRef = React.createRef<HTMLDivElement>();
    this.state = {
      isWide: false
    };
  }

  public componentDidMount(): void {
    this.resizeObserver = new ResizeObserver(this.onResize);
    if(this.containerRef.current) {
      this.resizeObserver.observe(this.containerRef.current);
    }
  }

  public componentWillUnmount(): void {
    this.resizeObserver?.disconnect();
  }

  public render(): JSX.Element {
    const hasDirection =
      this.props.delta.direction !== RequestsModel.Direction.NONE;
    return (
      <div ref={this.containerRef} className={css(STYLES.container)}>
        <div className={css(this.state.isWide ? STYLES.divWide : STYLES.div)}>
          <div className={
              css(this.state.isWide ? STYLES.headerWide : STYLES.header)}>
            {hasDirection &&
              <img aria-hidden='true' width='10' height='16'
                className={css(STYLES.tick)}
                src={this.props.delta.direction ===
                  RequestsModel.Direction.NEGATIVE ?
                  'resources/requests_page/downtick.svg' :
                  'resources/requests_page/uptick.svg'}/>}
            <h2 className={css(STYLES.name)}>{this.props.name}</h2>
          </div>
          <div className={css(STYLES.values)}>
            <span className={css(STYLES.oldValue)}>
              {this.props.oldValue}
            </span>
            <img aria-label='to' width='14' height='14'
              className={css(STYLES.arrow)}
              src='resources/requests_page/arrow-right.svg'/>
            <span className={css(STYLES.newValue)}>
              {this.props.newValue}
            </span>
          </div>
          {this.state.isWide &&
            <div className={css(STYLES.diffBadge)}>
              <DiffBadge
                value={this.props.delta.value}
                direction={this.props.delta.direction}/>
            </div>}
        </div>
      </div>);
  }

  private onResize = (entries: ResizeObserverEntry[]) => {
    for(const entry of entries) {
      const isWide = entry.contentRect.width >= 732;
      if(isWide !== this.state.isWide) {
        this.setState({isWide});
      }
    }
  };

  private containerRef: React.RefObject<HTMLDivElement>;
  private resizeObserver?: ResizeObserver;
}

const STYLES = StyleSheet.create({
  container: {
    fontFamily: 'Roboto'
  },
  div: {
    display: 'flex',
    alignItems: 'flex-start' as 'flex-start'
  },
  divWide: {
    display: 'inline-flex',
    alignItems: 'flex-start' as 'flex-start'
  },
  header: {
    display: 'flex',
    alignItems: 'flex-start' as 'flex-start',
    padding: '4px 0',
    flex: '1 1 auto',
    minWidth: 0
  },
  headerWide: {
    display: 'flex',
    alignItems: 'flex-start' as 'flex-start',
    padding: '4px 0',
    flex: '0 1 auto',
    minWidth: '128px',
    maxWidth: '48ch'
  },
  tick: {
    flexShrink: 0,
    marginRight: '4px'
  },
  name: {
    fontSize: '0.875rem',
    fontWeight: 400,
    color: '#333333',
    margin: 0,
    flex: '1 1 auto',
    minWidth: 0
  },
  values: {
    display: 'flex',
    alignItems: 'center',
    marginLeft: '18px',
    flexShrink: 0
  },
  oldValue: {
    fontSize: '0.875rem',
    color: '#333333',
    padding: '4px 0'
  },
  arrow: {
    margin: '0 4px',
    flexShrink: 0
  },
  newValue: {
    fontSize: '0.875rem',
    color: '#333333',
    padding: '4px 0'
  },
  diffBadge: {
    marginLeft: '18px',
    flexShrink: 0
  }
});

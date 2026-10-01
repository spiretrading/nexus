import * as Beam from 'beam';
import * as React from 'react';
import { DateRangeInput } from '../../components/date_range_input';
import { DateRangeOption } from '../../models/date_range';
import { makeParametersDateRangeOptions } from
  './parameters_date_range_options';

interface Properties extends Omit<React.ComponentProps<typeof DateRangeInput>,
    'label' | 'options' | 'orientation' | 'labelPosition' | 'boundsRequired'> {

  /** The reference date for presets. Defaults to the current local date. */
  today?: Beam.Date;
}

interface State {
  today: Beam.Date;
  inline: boolean;
}

/** Selects a report's date range using reporting presets or custom dates. */
export class ParametersDateRangeInput extends
    React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {today: Beam.Date.today(), inline: false};
    this.element = React.createRef();
    this.observer = null;
    this.timer = null;
    this.reference = null;
    this.options = [];
  }

  public render(): JSX.Element {
    const {today, style, ...properties} = this.props;
    const reference = today ?? this.state.today;
    if(!this.reference?.equals(reference)) {
      this.reference = reference;
      this.options = makeParametersDateRangeOptions(reference);
    }
    const labelPosition = (() => {
      if(this.state.inline) {
        return DateRangeInput.LabelPosition.INLINE;
      }
      return DateRangeInput.LabelPosition.ABOVE;
    })();
    return <div ref={this.element}
        style={{width: '100%', minWidth: 0, ...style}}>
      <DateRangeInput {...properties} label='Date Range'
        options={this.options} labelPosition={labelPosition}
        orientation={DateRangeInput.Orientation.VERTICAL} boundsRequired/>
    </div>;
  }

  public componentDidMount(): void {
    this.observer = new ResizeObserver(this.onResize);
    this.observer.observe(this.element.current);
    this.scheduleMidnight();
    window.addEventListener('focus', this.onTimeChange);
    document.addEventListener('visibilitychange', this.onTimeChange);
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.today !== this.props.today) {
      this.onTimeChange();
    }
  }

  public componentWillUnmount(): void {
    this.observer.disconnect();
    window.clearTimeout(this.timer);
    window.removeEventListener('focus', this.onTimeChange);
    document.removeEventListener('visibilitychange', this.onTimeChange);
  }

  private scheduleMidnight(): void {
    window.clearTimeout(this.timer);
    this.timer = null;
    if(!this.props.today) {
      const now = new Date();
      const next = new Date(now.getFullYear(), now.getMonth(),
        now.getDate() + 1);
      this.timer = window.setTimeout(this.onTimeChange,
        next.getTime() - now.getTime());
    }
  }

  private onResize = (entries: ResizeObserverEntry[]) => {
    const inline = entries[0].contentRect.width >= 384;
    if(inline !== this.state.inline) {
      this.setState({inline});
    }
  };

  private onTimeChange = () => {
    if(!this.props.today) {
      this.setState({today: Beam.Date.today()});
    }
    this.scheduleMidnight();
  };

  private element: React.RefObject<HTMLDivElement>;
  private observer: ResizeObserver;
  private timer: number;
  private reference: Beam.Date;
  private options: DateRangeOption[];
}

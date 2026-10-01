import * as Beam from 'beam';
import * as React from 'react';
import { DateRangeInput } from '../../../components/date_range_input';
import { DateRangeOption } from '../../../models/date_range';
import { makeDateFilterOptions } from './date_filter_options';

interface Properties extends Omit<React.ComponentProps<typeof DateRangeInput>,
    'options' | 'orientation' | 'labelPosition' | 'boundsRequired'> {

  /** The reference date for presets. Defaults to the current local date. */
  today?: Beam.Date;
}

interface State {
  today: Beam.Date;
  inline: boolean;
  horizontal: boolean;
}

/** Filters generated reports by calendar month or an optional custom range. */
export class DateFilter extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.inlineQuery = window.matchMedia('(min-width: 768px)');
    this.horizontalQuery = window.matchMedia('(min-width: 1036px)');
    this.state = {today: Beam.Date.today(), inline: this.inlineQuery.matches,
      horizontal: this.horizontalQuery.matches};
    this.timer = null;
    this.reference = null;
    this.options = [];
  }

  public render(): JSX.Element {
    const {today, ...properties} = this.props;
    const reference = today ?? this.state.today;
    if(!this.reference?.equals(reference)) {
      this.reference = reference;
      this.options = makeDateFilterOptions(reference);
    }
    const orientation = (() => {
      if(this.state.horizontal) {
        return DateRangeInput.Orientation.HORIZONTAL;
      }
      return DateRangeInput.Orientation.VERTICAL;
    })();
    const labelPosition = (() => {
      if(this.state.inline) {
        return DateRangeInput.LabelPosition.INLINE;
      }
      return DateRangeInput.LabelPosition.ABOVE;
    })();
    return <DateRangeInput {...properties} options={this.options}
      orientation={orientation} labelPosition={labelPosition}
      boundsRequired={false}/>;
  }

  public componentDidMount(): void {
    this.inlineQuery.addEventListener('change', this.onLayoutChange);
    this.horizontalQuery.addEventListener('change', this.onLayoutChange);
    this.onLayoutChange();
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
    this.inlineQuery.removeEventListener('change', this.onLayoutChange);
    this.horizontalQuery.removeEventListener('change', this.onLayoutChange);
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

  private onLayoutChange = () => {
    const inline = this.inlineQuery.matches;
    const horizontal = this.horizontalQuery.matches;
    if(inline !== this.state.inline || horizontal !== this.state.horizontal) {
      this.setState({inline, horizontal});
    }
  };

  private onTimeChange = () => {
    if(!this.props.today) {
      this.setState({today: Beam.Date.today()});
    }
    this.scheduleMidnight();
  };

  private inlineQuery: MediaQueryList;
  private horizontalQuery: MediaQueryList;
  private timer: number;
  private reference: Beam.Date;
  private options: DateRangeOption[];
}

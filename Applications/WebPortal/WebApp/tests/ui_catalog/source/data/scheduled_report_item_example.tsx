import * as React from 'react';
import { ScheduledReportItem } from 'web_portal';

interface Properties extends
    Omit<React.ComponentProps<typeof ScheduledReportItem>, 'runDate'> {

  /** The date edited by the catalog. */
  runDate: Date;

  /** Called instead of navigating away from the catalog. */
  onNavigate?: (href: string) => void;
}

/** Demonstrates a scheduled report without leaving the catalog on click. */
export class ScheduledReportItemExample extends React.Component<Properties> {
  public render(): JSX.Element {
    const {runDate, onNavigate, ...properties} = this.props;
    const date = {
      value: `${String(runDate.getFullYear()).padStart(4, '0')}-` +
        `${String(runDate.getMonth() + 1).padStart(2, '0')}-` +
        String(runDate.getDate()).padStart(2, '0'),
      label: runDate.toLocaleDateString('en-US', {
        month: 'short', day: '2-digit', year: 'numeric'
      })
    };
    return <ScheduledReportItem {...properties} runDate={date}
      onClick={this.onClick}/>;
  }

  private onClick = (event: React.MouseEvent<HTMLDivElement>) => {
    const link = (event.target as HTMLElement).closest('a');
    if(link) {
      event.preventDefault();
      this.props.onNavigate?.(link.getAttribute('href'));
    }
  };
}

interface ParameterProperties {

  /** The parameter being edited. */
  value: ScheduledReportItem.Parameter;

  /** Called when the parameter changes. */
  update: (value: ScheduledReportItem.Parameter) => void;
}

/** Edits a scheduled report parameter's label and displayed value. */
export class ScheduledReportParameterInput extends
    React.Component<ParameterProperties> {
  public render(): JSX.Element {
    return <div>
      <label>Label <input value={this.props.value.label}
        onChange={this.onLabel}/></label>
      <label>Value <input value={this.props.value.value}
        onChange={this.onValue}/></label>
    </div>;
  }

  private onLabel = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.props.update({...this.props.value, label: event.target.value});
  };

  private onValue = (event: React.ChangeEvent<HTMLInputElement>) => {
    this.props.update({...this.props.value, value: event.target.value});
  };
}

import * as React from 'react';
import { DisplaySize } from '../../..';
import { InputError } from './input_error';
import { InputValidation } from './input_validation';

interface Properties {

  /** The size of the element to display. */
  displaySize: DisplaySize;

  /** The name of the input. */
  label: string;

  /** The id of the input the label names. */
  htmlFor: string;

  /** The validation state of the input. */
  validation: InputValidation;

  /** Whether the input takes a fixed width rather than filling the row. */
  isLast?: boolean;
}

/** Displays a labelled input alongside its validation error. */
export class PropertyItem extends React.Component<Properties> {
  public render(): JSX.Element {
    const error = (
      <InputError
        displaySize={this.props.displaySize}
        label={this.props.label}
        validation={this.props.validation}/>);
    if(this.props.displaySize === DisplaySize.SMALL) {
      return (
        <div style={PropertyItem.STYLE.item}>
          <label
              style={PropertyItem.STYLE.smallLabel}
              htmlFor={this.props.htmlFor}>
            {this.props.label}
          </label>
          <div style={PropertyItem.STYLE.labelPadding}/>
          {this.props.children}
          {error}
        </div>);
    }
    const gap = (() => {
      if(this.props.displaySize === DisplaySize.MEDIUM) {
        return PropertyItem.STYLE.mediumGap;
      }
      return PropertyItem.STYLE.largeGap;
    })();
    const field = (() => {
      if(this.props.isLast) {
        return [
          <div key='item' style={PropertyItem.STYLE.fixedField}>
            {this.props.children}
          </div>,
          <div key='filler' style={PropertyItem.STYLE.filler}/>
        ];
      }
      return (
        <div style={PropertyItem.STYLE.field}>{this.props.children}</div>);
    })();
    return (
      <div style={PropertyItem.STYLE.item}>
        <div style={PropertyItem.STYLE.row}>
          <div style={PropertyItem.STYLE.labelColumn}>
            <div style={PropertyItem.STYLE.wrapper}>
              <div style={PropertyItem.STYLE.filler}/>
              <label
                  style={PropertyItem.STYLE.label}
                  htmlFor={this.props.htmlFor}>
                {this.props.label}
              </label>
              <div style={PropertyItem.STYLE.filler}/>
            </div>
            <div style={PropertyItem.STYLE.filler}/>
          </div>
          <div style={gap}/>
          {field}
        </div>
        {error}
      </div>);
  }

  private static readonly STYLE: Record<string, React.CSSProperties> = {
    item: {
      display: 'flex',
      flexDirection: 'column'
    },
    smallLabel: {
      fontSize: '0.875rem',
      paddingInlineStart: '10px'
    },
    labelPadding: {
      height: '12px',
      flexShrink: 0
    },
    row: {
      display: 'flex',
      flexDirection: 'row'
    },
    labelColumn: {
      width: '130px',
      minWidth: '130px',
      flexShrink: 0,
      display: 'flex',
      flexDirection: 'column'
    },
    wrapper: {
      height: '34px',
      flexShrink: 0,
      display: 'flex',
      flexDirection: 'column'
    },
    label: {
      fontSize: '0.875rem'
    },
    filler: {
      flexGrow: 1
    },
    mediumGap: {
      width: '8px',
      minWidth: '8px',
      flexShrink: 0
    },
    largeGap: {
      width: '40px',
      minWidth: '40px',
      flexShrink: 0
    },
    field: {
      flexGrow: 1,
      minWidth: 0
    },
    fixedField: {
      width: '200px',
      minWidth: '200px',
      flexShrink: 0
    }
  };
}

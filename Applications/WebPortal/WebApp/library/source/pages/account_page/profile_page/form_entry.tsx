import * as React from 'react';
import { HBoxLayout, VBoxLayout, Padding } from '../../../layouts';

interface Properties {

  /** The name of the input field. */
  name: string;

  /** Determines if the input is editable. */
  readonly?: boolean;

  /** Whether the form is rendered vertically or horizontally. */
  orientation: FormEntry.Orientation;

  /** The input field to render. */
  children: JSX.Element;
}

/** Displays an input form for a single item. */
export class FormEntry extends React.Component<Properties> {
  public static readonly defaultProps = {
    readonly: false
  }

  constructor(props: Properties) {
    super(props);
  }

  public render(): JSX.Element {
    const boxStyle = (() => {
      if(this.props.readonly) {
        return FormEntry.STYLE.box;
      } else {
        return null;
      }
    })();
    const content = (() => {
      if(this.props.orientation === FormEntry.Orientation.HORIZONTAL) {
        return (
          <HBoxLayout style={boxStyle}>
            <div style={FormEntry.STYLE.horizontalHeader}>
              {this.props.name}
            </div>
            <Padding size={FormEntry.HORIZONTAL_PADDING}/>
            <div>{this.props.children}</div>
          </HBoxLayout>);
      } else {
        return (
          <VBoxLayout style={boxStyle} width='100%'>
            <div style={FormEntry.STYLE.verticalHeader}>{this.props.name}</div>
            <Padding size={FormEntry.VERTICAL_PADDING}/>
            {this.props.children}
          </VBoxLayout>);
      }
    })();
    return (
      <div>
        {content}
      </div>);
  }

  private static STYLE = {
    box: {
      cursor: 'default'
    },
    childBox: {
      width: '100%'
    },
    horizontalHeader: {
      height: '34px',
      width: '130px',
      font: '400 14px Roboto',
      color: '#000000',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center'
    },
    verticalHeader : {
      height: '16px',
      font: '400 14px Roboto',
      color: '#000000',
      paddingLeft: '10px',
      width: '100%'
    }
  };
  private static readonly VERTICAL_PADDING = '12px';
  private static readonly HORIZONTAL_PADDING = '8px';
}

export namespace FormEntry {

  /** Lists the ways that a form can be displayed. */
  export enum Orientation {

    /** The form is rendered horizontally. */
    HORIZONTAL,

    /** The form is rendered vertically. */
    VERTICAL
  }
}

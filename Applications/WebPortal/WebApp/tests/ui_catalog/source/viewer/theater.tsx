import * as React from 'react';
import { PropertiesList } from './property_list';
import { ComponentSchema } from '../data/schemas';
import { Stage } from './stage';

interface Properties {

  /** The component to display. */
  component?: ComponentSchema;

  /** The props for the component. */
  componentProps?: {[name: string]: any};

  /** The signals that were sent by the component.*/
  log: string[];

  /** The callback to update a prop.
   *@param name - The name of the prop.
   *@param newValue - The new value for the prop.
   */
  update: (name: string, newValue: any) => void;
}

/** Container component that holds the component and the property list. */
export class Theater extends React.Component<Properties> {
  public render(): JSX.Element {
    if(this.props.component === null) {
      return (
        <div style={Theater.STYLE.noSelection}>
          Select a component.
        </div>);
    }
    return (
      <div style={Theater.STYLE.container}>
        <div style={Theater.STYLE.stageWrapper}>
          <div style={Theater.STYLE.stage}>
            <Stage component={this.props.component}
              values={this.props.componentProps}/>
          </div>
          <div style={Theater.STYLE.signalContainer}>
            <div style={Theater.STYLE.signalHeader}>Signals</div>
            <div style={Theater.STYLE.signalBox}>
              {this.props.log.join('\n')}
            </div>
          </div>
        </div>
        <PropertiesList 
          properties={this.props.component.properties}
          values={this.props.componentProps}
          updateValue={this.props.update}/>
      </div>);
  }

  private static readonly STYLE = {
    container: {
      width: '100%',
      height: '100%',
      display: 'flex',
      flexDirection: 'row'
    } as React.CSSProperties,
    noSelection: {
      width: '100%',
      height: '100%',
      display: 'flex',
      flexDirection: 'row',
      justifyContent: 'center',
      alignItems: 'center',
      fontSize: '20px',
      color: '#000000',
      fontWeight: 700
    } as React.CSSProperties,
    stageWrapper: {
      height: '100%',
      width: '100%',
      display: 'flex',
      flexDirection: 'column'
    } as React.CSSProperties,
    stage: {
      height: '66%',
      overflow: 'auto'
    } as React.CSSProperties,
    signalContainer: {
      boxSizing: 'border-box',
      height: '34%',
      backgroundColor: '#F5F5F5',
      padding: '22px 20px 20px 20px',
      display: 'flex',
      flexDirection: 'column'
    } as React.CSSProperties,
    signalHeader: {
      fontSize: '20px',
      color: '#000000',
      fontWeight: 700,
      flexGrow: 0,
      flexShrink: 0
    } as React.CSSProperties,
    signalBox: {
      whiteSpace: 'pre-line',
      boxSizing: 'border-box',
      padding: '15px 13px 15px 13px',
      marginTop: '20px',
      borderStyle: 'solid',
      borderWidth: '1px',
      borderColor: '#C8C8C8',
      backgroundColor: '#FFFFFF',
      lineHeight: '21px',
      fontSize: '14px',
      fontWeight: 400,
      color: '#000000',
      display: 'flex',
      flexDirection: 'column',
      overflowY: 'scroll',
      overflowWrap: 'anywhere',
      flexGrow: 1,
      flexShrink: 1
    } as React.CSSProperties
  };
}

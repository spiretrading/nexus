import * as React from 'react';
import { PropertySchema } from '../data';
import { NumberSliderInput } from './propertyInput/number_slider_input';

interface Properties {

  /** A list of properties. */
  properties: PropertySchema[];

  /** The current values for each property. */
  values: {[name: string]: any};

  /** The callback to update values. */
  updateValue: (name: string, value: any) => void;

  /** Whether to show a border around the component. */
  showBorder: boolean;

  /** Callback to toggle the border. */
  onToggleBorder: () => void;

  /** The container width (-1 for preferred size). */
  containerWidth: number;

  /** The container height (-1 for preferred size). */
  containerHeight: number;

  /** Callback to update the container width. */
  onContainerWidthChange: (width: number) => void;

  /** Callback to update the container height. */
  onContainerHeightChange: (height: number) => void;
}

/** Displays a list of properties. */
export class PropertiesList extends React.Component<Properties> {
  render(): JSX.Element {
    return (
      <div style={PropertiesList.STYLE.container}>
        <div style={PropertiesList.STYLE.header}>Properties</div>
        <div style={PropertiesList.STYLE.propertyContainer}>
          <label style={PropertiesList.STYLE.checkboxLabel}>
            <input type='checkbox'
              checked={this.props.showBorder}
              onChange={this.props.onToggleBorder}/>
            Show Border
          </label>
        </div>
        <div style={PropertiesList.STYLE.propertyContainer}>
          <div style={PropertiesList.STYLE.label}>Container Width</div>
          <NumberSliderInput
            value={this.props.containerWidth}
            min={-1} max={2000}
            update={this.props.onContainerWidthChange}/>
        </div>
        <div style={PropertiesList.STYLE.propertyContainer}>
          <div style={PropertiesList.STYLE.label}>Container Height</div>
          <NumberSliderInput
            value={this.props.containerHeight}
            min={-1} max={2000}
            update={this.props.onContainerHeightChange}/>
        </div>
        {this.props.properties.map(this.renderItem)}
      </div>);
  }

  private renderItem = (property: PropertySchema) => {
    return (
      <div key={property.name} style={PropertiesList.STYLE.propertyContainer}>
        <div style={PropertiesList.STYLE.label}>{property.name}</div>
        <property.render
          value={this.props.values[property.name]}
          update={(newValue: any) => 
            {this.props.updateValue(property.name, newValue)}}/>
      </div>);
  }

  private static readonly STYLE = {
    container: {
      display: 'flex',
      flexDirection: 'column',
      color: '#000000',
      flexShrink: 0,
      flexGrow: 0,
      backgroundColor: '#FFFFFF',
      padding: '22px 20px 20px 20px',
      overflowY: 'auto'
    } as React.CSSProperties,
    header: {
      fontSize: '20px',
      fontWeight: 700,
      fontFamily: 'Roboto',
      paddingBottom: '30px'
    } as React.CSSProperties,
    propertyContainer: {
      display: 'flex',
      flexDirection: 'column',
      padding: '0 0 18px 0',
    } as React.CSSProperties,
    label: {
      fontSize: '15px',
      fontWeight: 600,
      fontFamily: 'Roboto',
      color: '#000000',
      paddingBottom: '11px'
    } as React.CSSProperties,
    checkboxLabel: {
      display: 'flex',
      alignItems: 'center',
      gap: '6px',
      fontSize: '14px',
      fontFamily: 'Roboto',
      color: '#000000',
      cursor: 'pointer'
    } as React.CSSProperties
  };
}

import * as React from 'react';
import { PropertySchema } from '../data';

interface Properties {

  /** A list of properties. */
  properties: PropertySchema[];

  /** The current values for each property. */
  values: {[name: string]: any};

  /** The callback to update values. */
  updateValue: (name: string, value: any) => void;
}

/** Displays a list of properties. */
export class PropertiesList extends React.Component<Properties> {
  render(): JSX.Element {
    return (
      <div style={PropertiesList.STYLE.container}>
        <div style={PropertiesList.STYLE.header}>Properties</div>
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
    } as React.CSSProperties
  };
}

import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { ComponentSchema } from '../data/schemas';

interface Properties {

  /** The list of all available components. */
  componentList: ComponentSchema[];

  /** The selected component. */
  selected?: ComponentSchema;

  /** Callback to update the selected component. */
  onClick?: (component: ComponentSchema) => void;
}

/** Displays a list of components to select from. */
export class ComponentList extends React.Component<Properties> {
  public render(): JSX.Element {
    return (
      <div style={ComponentList.STYLE.container}>
        <span style={ComponentList.STYLE.header}>Components</span>
        <div>{this.props.componentList.map(this.renderItem)}</div>
      </div>);
  }

  private renderItem = (component: ComponentSchema) => {
    const isSelected = component.name === this.props.selected?.name;
    return (
      <button
          key={component.name}
          onClick={() => this.props.onClick(component)}
          className={css(ComponentList.EXTRA_STYLE.effects)}
          style={{...ComponentList.STYLE.item,
            ...(isSelected && ComponentList.STYLE.selected)}}>
        {component.name}
      </button>);
  }

  private static readonly STYLE = {
    container: {
      width: '240px',
      overflowY: 'auto',
      display: 'flex',
      flexDirection: 'column',
      color: 'white',
      backgroundColor: '#4B23A0',
      paddingTop: '22px',
      flexGrow: 0,
      flexShrink: 0
    } as React.CSSProperties,
    header: {
      fontSize: '20px',
      fontWeight: 700,
      fontFamily: 'Roboto',
      color: '#FFFFFF',
      flexGrow: 0,
      flexShrink: 0,
      padding: '0 20px 30px 20px'
    } as React.CSSProperties,
    item: {
      width: '100%',
      height: '40px',
      padding: '11px 20px 11px 20px',
      fontFamily: 'Roboto',
      fontSize: '14px',
      color: '#FFFFFF',
      backgroundColor: 'transparent',
      border: 'none',
      display: 'flex',
      justifyContent: 'start',
      cursor: 'pointer',
      appearance: 'none'
    } as React.CSSProperties,
    selected: {
      backgroundColor: '#6B3FC0'
    } as React.CSSProperties
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    effects: {
      ':hover': {
        backgroundColor: '#5C33B0'
      },
      ':focus': {
        outline: '-webkit-focus-ring-color auto 0px'
      }
    }
  });
}

import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { ComponentSchema, ComponentSection } from '../data/schemas';

interface Properties {

  /** The list of component sections. */
  sections: ComponentSection[];

  /** The selected component. */
  selected?: ComponentSchema;

  /** Callback to update the selected component. */
  onClick?: (component: ComponentSchema) => void;
}

interface State {
  collapsed: {[name: string]: boolean};
}

/** Displays a list of components to select from, grouped by section. */
export class ComponentList extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {collapsed: {}};
  }

  public render(): JSX.Element {
    return (
      <div style={ComponentList.STYLE.container}>
        <span style={ComponentList.STYLE.header}>Components</span>
        {this.props.sections.map(this.renderSection)}
      </div>);
  }

  private renderSection = (section: ComponentSection) => {
    const isCollapsed = this.state.collapsed[section.name] ?? false;
    return (
      <div key={section.name}>
        <button
            style={ComponentList.STYLE.sectionHeader}
            className={css(ComponentList.EXTRA_STYLE.sectionEffects)}
            onClick={() => this.toggleSection(section.name)}>
          <span style={isCollapsed ?
            ComponentList.STYLE.arrowCollapsed :
            ComponentList.STYLE.arrowExpanded}>{'\u25B6'}</span>
          {section.name}
        </button>
        {!isCollapsed &&
          section.components.map(this.renderItem)}
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

  private toggleSection = (name: string) => {
    this.setState(state => ({
      collapsed: {
        ...state.collapsed,
        [name]: !(state.collapsed[name] ?? false)
      }
    }));
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
    sectionHeader: {
      width: '100%',
      padding: '11px 20px',
      fontFamily: 'Roboto',
      fontSize: '12px',
      fontWeight: 700,
      color: '#C7BAFF',
      backgroundColor: 'transparent',
      border: 'none',
      display: 'flex',
      alignItems: 'center',
      gap: '8px',
      cursor: 'pointer',
      appearance: 'none',
      textTransform: 'uppercase',
      letterSpacing: '0.05em'
    } as React.CSSProperties,
    arrowExpanded: {
      fontSize: '8px',
      transition: 'transform 0.15s',
      transform: 'rotate(90deg)'
    } as React.CSSProperties,
    arrowCollapsed: {
      fontSize: '8px',
      transition: 'transform 0.15s',
      transform: 'rotate(0deg)'
    } as React.CSSProperties,
    item: {
      width: '100%',
      height: '40px',
      padding: '11px 20px 11px 32px',
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
    },
    sectionEffects: {
      ':hover': {
        backgroundColor: '#5C33B0'
      },
      ':focus': {
        outline: '-webkit-focus-ring-color auto 0px'
      }
    }
  });
}

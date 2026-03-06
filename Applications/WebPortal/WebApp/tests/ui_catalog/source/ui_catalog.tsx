import * as React from 'react';
import { ComponentSchema, ComponentSection } from './data';
import { ComponentList, Theater } from './viewer';

interface Properties {

  /** The list of component sections. */
  sections: ComponentSection[];
}

interface State {
  component: ComponentSchema;
  componentValues: {[name: string]: any};
  log: string[];
  showBorder: boolean;
  backgroundColor: string;
  containerWidth: number;
  containerHeight: number;
}

/** The app to display and demo components. */
export class UICatalog extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      component: null,
      componentValues: {},
      log: [],
      showBorder: false,
      backgroundColor: '#E8E8E8',
      containerWidth: -1,
      containerHeight: -1
    };
  }

  public render(): JSX.Element {
    return (
      <div style={UICatalog.STYLE.page}>
        <ComponentList
          sections={this.props.sections}
          selected={this.state.component}
          onClick={this.componentChange}/>
        <Theater
          component={this.state.component}
          componentProps={this.state.componentValues}
          log={this.state.log}
          update={this.updateValue}
          showBorder={this.state.showBorder}
          onToggleBorder={this.toggleBorder}
          backgroundColor={this.state.backgroundColor}
          onBackgroundColorChange={this.updateBackgroundColor}
          containerWidth={this.state.containerWidth}
          containerHeight={this.state.containerHeight}
          onContainerWidthChange={this.updateContainerWidth}
          onContainerHeightChange={this.updateContainerHeight}/>
      </div>);
  }

  private componentChange = (component: ComponentSchema) => {
    const newValues = {} as {[name: string]: any};
    for(const property of component.properties) {
      newValues[property.name] = property.defaultValue;
    }
    for(const signal of component.signals) {
      if (signal.parameters.length !== 0) {
        const localParameters = [] as any[];
        const localProperties = [] as string[];
        for(let i = 0; i < signal.parameters.length; i++) {
          localProperties.push(signal.parameters[i].propertyName);
          localParameters.push(signal.parameters[i].parameterName);
        }
        newValues[signal.name] = (...localParameters: any) =>
          this.onMixedSignal(signal.name, localProperties, ...localParameters);
      } else if(signal.propertyName === '') {
        newValues[signal.name] = () =>
          this.onSignal(signal.name);
      } else {
        newValues[signal.name] = (newValue: any) =>
          this.onChange(signal.name, signal.propertyName, newValue);
      }
    }
    this.setState({
      component: component,
      componentValues: newValues,
      log: [],
      containerWidth: component.defaultContainerWidth,
      containerHeight: component.defaultContainerHeight
    });
  }

  private toggleBorder = () => {
    this.setState((state: State) => ({showBorder: !state.showBorder}));
  }

  private updateBackgroundColor = (color: string) => {
    this.setState({backgroundColor: color});
  }

  private updateContainerWidth = (width: number) => {
    this.setState({containerWidth: width});
  }

  private updateContainerHeight = (height: number) => {
    this.setState({containerHeight: height});
  }

  private updateValue = (name: string, newValue: any) => {
    this.setState((state: State) => {
      this.state.componentValues[name] = newValue;
      return {componentValues: state.componentValues};
    });
  }

  private onChange = (signalName: string, property: string, newValue: any) => {
    this.updateValue(property, newValue);
    this.setState((state: State) => {
      const lineNumber = state.log.length;
      const newValueString = JSON.stringify(newValue);
      state.log.unshift(`${lineNumber}: ${signalName} (${newValueString})`);
      return {log: state.log};
    });
  }

  private onSignal = (signalOutput: string) => {
    this.setState((state: State) => {
      const lineNumber = state.log.length;
      state.log.unshift(`${lineNumber}: ${signalOutput}`);
      return {log: state.log};
    });
  }

  private onMixedSignal = (signalName: string, properties: string[],
      ...parameters: any[]) => {
    for(let i = 0; i < parameters.length; i++) {
      if(properties[i] !== '') {
        this.updateValue(properties[i], parameters[i]);
      }
    }
    this.setState((state: State) => {
      const lineNumber = state.log.length;
      state.log.unshift(`${lineNumber}: ${signalName}`);
      return {log: state.log};
    });
  }

  private static readonly STYLE = {
    page: {
      width: '100%',
      height: '100%',
      display: 'flex',
      flexDirection: 'row',
      backgroundColor: '#E8E8E8'
    } as React.CSSProperties
  };
}

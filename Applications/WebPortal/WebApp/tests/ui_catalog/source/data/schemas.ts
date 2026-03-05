import * as React from 'react';

type Renderable =
  typeof React.Component | React.FunctionComponent | ((_: any) => JSX.Element);

/** The data model for the component. */
export class ComponentSchema {

  /** Constructs a ComponentSchema
   * @param name - The name of the component.
   * @param properties[] - The properties associated with the component.
   * @param signals[] - The signals of the component.
   * @param render - The component to render.
   */
  constructor(name: string, properties: PropertySchema[],
      signals: SignalSchema[], render: Renderable) {
    this._name = name;
    this._properties = properties.slice();
    this._signals = signals.slice();
    this._render = render;
  }

  /** Returns the name of the component. */
  public get name(): string {
    return this._name;
  }

  /** Returns the properties of the component. */
  public get properties(): PropertySchema[] {
    return this._properties.slice();
  }

  /** Returns the signals associated with the component. */
  public get signals(): SignalSchema[] {
    return this._signals.slice();
  }

  /** Returns the component associated with the schema. */
  public get render(): Renderable {
    return this._render;
  }

  private _name: string;
  private _properties: PropertySchema[];
  private _signals: SignalSchema[]
  private _render: Renderable;
}

/** The data model for the property. */
export class PropertySchema {

  /** Constructs a PropertySchema.
   * @param name - The name of the property.
   * @param defaultValue - The default value of the property.
   * @param render - The input field for the property.
   */
  constructor(name: string, defaultValue: any, render: Renderable) {
    this._name = name;
    this._defaultValue = defaultValue;
    this._render = render;
  }

  /** Returns the name of the property. */
  public get name(): string {
    return this._name;
  }

  /** Returns the default value of the property. */
  public get defaultValue(): any  {
    return this._defaultValue;
  }

  /** Returns the input field for the property. */
  public get render(): Renderable {
    return this._render;
  }

  private _name: string;
  private _defaultValue: any;
  private _render: Renderable;
}

export interface Parameter {
  
  /** The name of the parameter. */
  parameterName: string;

  /** The name of the property that gets modified by this parameter. Make it
    * a empty string if there is no property. */
  propertyName: string;
}

/** The data model for the signal.*/
export class SignalSchema {

  /** Constructs a SignalSchema.
   * @param name - The name of the signal.
   * @param propertyName - The name of property the signal updates.
   * @param parameters - The list of parameters. Use if the component has more
   * than one parameter.
   */
  constructor(name: string, propertyName: string, parameters?: Parameter[]) {
    this._name = name;
    this._propertyName = propertyName;
    if(parameters === undefined) {
      this._parameters = [];
    } else {
      this._parameters = parameters?.slice();
    }
  }

  /** Returns the name of the signal. */
  public get name(): string {
    return this._name;
  }

  /** Returns the name of property the signal updates. */
  public get propertyName(): string {
    return this._propertyName;
  }

  /** Returns a list of Parameters. */
  public get parameters(): Parameter[] {
    return this._parameters.slice();
  }

  private _name: string;
  private _propertyName: string;
  private _parameters: Parameter[];
}

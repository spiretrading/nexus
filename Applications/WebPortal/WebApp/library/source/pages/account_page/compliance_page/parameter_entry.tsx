import * as React from 'react';
import * as Nexus from 'nexus';
import * as Beam from 'beam';
import { DisplaySize } from '../../../display_size';
import {CurrencySelectionBox, MoneyInputBox, TextInputField} from '../../../components';

interface Properties {
  displaySize: DisplaySize;
  parameter?: Nexus.ComplianceParameter;
  currencyDatabase?: Nexus.CurrencyDatabase;
  onChange?: (newValue: any) => void
}

export class ParameterEntry extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
  }

  public render(): JSX.Element {
    const input = (() => {
      switch(this.props.parameter.value.type) {
        case Nexus.ComplianceValue.Type.BOOLEAN:
          return <div/>;
        case Nexus.ComplianceValue.Type.CURRENCY:
          return (
            <CurrencySelectionBox 
              currencyDatabase={this.props.currencyDatabase}/>);
        case Nexus.ComplianceValue.Type.DATE_TIME:
          return <div/>;
        case Nexus.ComplianceValue.Type.DOUBLE:
          return <div/>;
        case Nexus.ComplianceValue.Type.DURATION:
          return <div/>;
        case Nexus.ComplianceValue.Type.MONEY:
          return <MoneyInputBox value={this.props.parameter.value.value}/>;
        case Nexus.ComplianceValue.Type.QUANTITY:
          return <div/>;
        case Nexus.ComplianceValue.Type.SECURITY:
          return <div/>;
        case Nexus.ComplianceValue.Type.LIST:
          const list = [] as any[];
          for(const thing of this.props.parameter.value.value) {
            list.push(
              <ParameterEntry 
                displaySize={this.props.displaySize}
                parameter={thing}/>);
          }
        case Nexus.ComplianceValue.Type.NONE:
          return <div/>
      }
    })();
    return (
      <div>
        <div>{'Label'}</div>
        {input}
      </div>
    );
  }
}


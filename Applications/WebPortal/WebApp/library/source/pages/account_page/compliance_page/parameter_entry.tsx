import * as React from 'react';
import * as Nexus from 'nexus';
import * as Beam from 'beam';
import { DisplaySize } from '../../../display_size';
import {CurrencySelectionBox, MoneyInputBox, TextInputField} from '../../../components';

interface Properties {
  displaySize: DisplaySize;
  name: string,
  type: Nexus.ComplianceValue,
  value: any,
  onChange?: (newValue: any) => void
}

export class ParameterEntry extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
  }

  public render(): JSX.Element {
    const input = (() => {
      switch(this.props.type.type) {
        case Nexus.ComplianceValue.Type.BOOLEAN:
          return <div/>;
        case Nexus.ComplianceValue.Type.CURRENCY:
          return <div/>
        case Nexus.ComplianceValue.Type.DATE_TIME:
          return <div/>;
        case Nexus.ComplianceValue.Type.DOUBLE:
          return <div/>;
        case Nexus.ComplianceValue.Type.DURATION:
          return <div/>;
        case Nexus.ComplianceValue.Type.MONEY:
          return <MoneyInputBox value={this.props.value}/>;
        case Nexus.ComplianceValue.Type.QUANTITY:
          return <div/>;
        case Nexus.ComplianceValue.Type.SECURITY:
          return <div/>;
        case Nexus.ComplianceValue.Type.LIST:
          return <div/>;
        case Nexus.ComplianceValue.Type.NONE:
          return <div/>
      }
    })();
    return (
      <div>
        <div>{this.props.name}</div>
        {input}
      </div>
    );
  }
}


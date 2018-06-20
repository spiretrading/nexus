import * as React from 'react';
import * as ReactDOM from 'react-dom';
import {BurgerButton, Padding, VBoxLayout, HBoxLayout} from 'web_portal';

interface State {
  openSideBars: number[]
}

export class App extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {
      openSideBars: []
    };
  }
  public render(): JSX.Element {
    
    return (
      <VBoxLayout>
        <VBoxLayout height='44px' width='320px'>
          <Padding size='15px'/>
          <HBoxLayout width='136px'>
            <Padding size='18px'/>
            <BurgerButton width='20px' height='14px' color='green'
                highlightColor='yellow' onClick={()=> console.log('onclick')}/>
          </HBoxLayout>
          <Padding size='15px'/>
        </VBoxLayout>
        <Padding size='50px'/>
        <VBoxLayout height='60px' width='320px'>
          <Padding size='15px'/>
          <HBoxLayout width='136px'>
            <Padding size='18px'/>
            <BurgerButton width='40px' height='34px' color='#684BC7'
                highlightColor='#FFFFFF' onClick={()=> console.log('onclick')}/>
          </HBoxLayout>
          <Padding size='15px'/>
        </VBoxLayout>
        <Padding size='50px'/>
        <VBoxLayout height='90px' width='320px'>
          <Padding size='15px'/>
          <HBoxLayout width='136px'>
            <Padding size='18px'/>
            <BurgerButton width='60px' height='60px' color='pink'
                highlightColor='blue' onClick={()=> console.log('onclick')}/>
          </HBoxLayout>
          <Padding size='15px'/>
        </VBoxLayout>
        <Padding size='50px'/>
        <VBoxLayout height='120px' width='320px'>
          <Padding size='15px'/>
          <HBoxLayout width='136px'>
            <Padding size='18px'/>
            <BurgerButton width='20px' height='90px' color='orange'
                highlightColor='green' onClick={()=> console.log('onclick')}/>
          </HBoxLayout>
        </VBoxLayout>
      </VBoxLayout>
      )
  }
}
ReactDOM.render(<App/>, document.getElementById('main'));

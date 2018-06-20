import * as React from 'react';
import * as ReactDOM from 'react-dom';
import {BurgerButton, NotificationButton, HBoxLayout, Padding,
	VBoxLayout} from 'web_portal';

export class App extends React.Component<{}, {}> {
  public render(): JSX.Element {
    return (
      <VBoxLayout>
        <VBoxLayout height='44px' width='320px'>
          <Padding size='15px'/>
          <HBoxLayout width='136px'>
            <Padding size='18px'/>
            <BurgerButton width='20px' height='14px' color='green'
              highlightColor='yellow' onClick={() => console.log(
              	'onclick burger button')}/>
          </HBoxLayout>
          <Padding size='15px'/>
        </VBoxLayout>
        <Padding size='50px'/>
        <VBoxLayout height='60px' width='320px'>
          <Padding size='15px'/>
          <HBoxLayout width='136px'>
            <Padding size='18px'/>
            <BurgerButton width='40px' height='34px' color='#684BC7'
              highlightColor='#FFFFFF' onClick={() => console.log(
              	'onclick burger button')}/>
          </HBoxLayout>
          <Padding size='15px'/>
        </VBoxLayout>
        <Padding size='50px'/>
        <VBoxLayout height='90px' width='320px'>
          <Padding size='15px'/>
          <HBoxLayout width='136px'>
            <Padding size='18px'/>
            <BurgerButton width='60px' height='60px' color='pink'
              highlightColor='blue' onClick={() => console.log(
              	'onclick burger button')}/>
          </HBoxLayout>
          <Padding size='15px'/>
        </VBoxLayout>
        <Padding size='50px'/>
        <VBoxLayout height='120px' width='320px'>
          <Padding size='15px'/>
          <HBoxLayout width='136px'>
            <Padding size='18px'/>
            <BurgerButton width='20px' height='90px' color='orange'
              highlightColor='green' onClick={() => console.log(
              	'onclick burger button')}/>
          </HBoxLayout>
        </VBoxLayout>
        <Padding size='50px'/>
        <NotificationButton width='45px' height='45px'
        	onClick={() => console.log('onclick notification button')}/>
        <NotificationButton width='20px' height='20px'
        	onClick={() => console.log('onclick notification button')}/>
        <NotificationButton width={30} height={30}
        	onClick={() => console.log('onclick notification button')}/>
      </VBoxLayout>);
  }
}

ReactDOM.render(<App/>, document.getElementById('main'));

import{css, StyleSheet} from 'aphrodite';
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
		const header1ClassName = (() => {
			if(this.state.openSideBars.includes(1)) {
				return css(App.STYLE.highlightedHeader1)
			}
		})();
		const header2ClassName = (() => {
			if(this.state.openSideBars.includes(2)) {
				return css(App.STYLE.highlightedHeader2)
			}
		})();
		const header3ClassName = (() => {
			if(this.state.openSideBars.includes(3)) {
				return css(App.STYLE.highlightedHeader3)
			}
		})();
		const header4ClassName = (() => {
			if(this.state.openSideBars.includes(4)) {
				return css(App.STYLE.highlightedHeader4)
			}
		})();
		const onBurgerButtonClick = (num: number) => {
			let newSideBar;
			newSideBar =  this.state.openSideBars.slice();
			const index = this.state.openSideBars.indexOf(num)
			if(index < 0) {
				newSideBar.push(num)
			} else {
				newSideBar.splice(index, 1);
			}
			this.setState({openSideBars: newSideBar});
		}
		return (
			<VBoxLayout>
				<VBoxLayout height='44px' width='320px' className={header1ClassName}>
          <Padding size='15px'/>
          <HBoxLayout width='136px'>
            <Padding size='18px'/>
            <BurgerButton width='20px' height='14px' color='green'
                highlightColor='#FFFFFF' onClick={()=> onBurgerButtonClick(1)}/>
          </HBoxLayout>
          <Padding size='15px'/>
        </VBoxLayout>
        <Padding size='50px'/>
        <VBoxLayout height='60px' width='320px' className={header2ClassName}>
          <Padding size='15px'/>
          <HBoxLayout width='136px'>
            <Padding size='18px'/>
            <BurgerButton width='40px' height='34px' color='#684BC7'
                highlightColor='#FFFFFF' onClick={()=> onBurgerButtonClick(2)}/>
          </HBoxLayout>
          <Padding size='15px'/>
        </VBoxLayout>
        <Padding size='50px'/>
        <VBoxLayout height='90px' width='320px' className={header3ClassName}>
          <Padding size='15px'/>
          <HBoxLayout width='136px'>
            <Padding size='18px'/>
            <BurgerButton width='60px' height='60px' color='pink'
                highlightColor='#FFFFFF' onClick={()=> onBurgerButtonClick(3)}/>
          </HBoxLayout>
          <Padding size='15px'/>
        </VBoxLayout>
        <Padding size='50px'/>
        <VBoxLayout height='120px' width='320px' className={header4ClassName}>
          <Padding size='15px'/>
          <HBoxLayout width='136px'>
            <Padding size='18px'/>
            <BurgerButton width='20px' height='90px' color='orange'
                highlightColor='#FFFFFF' onClick={()=> onBurgerButtonClick(4)}/>
          </HBoxLayout>
        </VBoxLayout>
			</VBoxLayout>
			)
	}
	private static STYLE = StyleSheet.create({
    defaultHeader: {
      backgroundColor: '#FFFFF'
    },
    highlightedHeader1: {
      backgroundColor: 'green'
    },
    highlightedHeader2: {
      backgroundColor: '#4B23A0'
    },
    highlightedHeader3: {
      backgroundColor: 'pink'
    },
    highlightedHeader4: {
      backgroundColor: 'orange'
    },
  });
}
ReactDOM.render(<App/>, document.getElementById('main'));

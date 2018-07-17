import {css, StyleSheet} from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';
import {Center, HBoxLayout, Padding, VBoxLayout} from '../../..';

interface Properties {

  /** The parameters to display. */
  parameters: Nexus.RiskParameters;

  /** Used to lookup currency names and symbols. */
  currencyDatabase: Nexus.CurrencyDatabase;
}

interface FooterProperties {
	isAdmin: boolean;
}

interface NumberInputBoxProperties {
	label: string;
	onChange: (value: number) => void;
}

interface NumberInputBoxState {
	value: string;
}

interface State {
	breakpoint: Breakpoint;
}

enum Breakpoint {
  SMALL,
  MEDIUM,
  LARGE
}

/** Implements a React component to display a set of RiskParameters. */
export class RiskParametersView extends React.Component<Properties, State> {

	public constructor(props: Properties) {
    super(props);
    this.state = {
      breakpoint: RiskParametersView.getBreakpoint()
    };
    this.onScreenResize = this.onScreenResize.bind(this);
  }

  public componentDidMount() {
    window.addEventListener('resize', this.onScreenResize);
  }

  public componentWillUnmount(): void {
    window.removeEventListener('resize', this.onScreenResize);
  }

  public render(): JSX.Element {
  	const containerClassName = (() => {
  		switch(this.state.breakpoint) {
  			case Breakpoint.SMALL:
  				return css(RiskParametersView.STYLE.smallContainer);
  			case Breakpoint.MEDIUM:
  				return css(RiskParametersView.STYLE.mediumContainer);
  			case Breakpoint.LARGE:
  				return css(RiskParametersView.STYLE.largeContainer);
  			default:
  				Breakpoint.MEDIUM;
  		}
  	})();
    return (
    		<HBoxLayout width='100%' height='100%'>
    			<Padding/>
    			<VBoxLayout className={containerClassName}>
    				<VBoxLayout width='246px' height='359px'>
    				<NumberInputBox label='Buying Power ($)'
    					onChange={this.onBuyingPowerChange}/>
    				</VBoxLayout>
    			</VBoxLayout>
    			<Padding/>
    		</HBoxLayout>
    	);
  }

  private static getBreakpoint(): Breakpoint {
    const screenWidth = window.innerWidth ||
      document.documentElement.clientWidth ||
      document.getElementsByTagName('body')[0].clientWidth;
    if(screenWidth <= 767) {
      return Breakpoint.SMALL;
    } else if(screenWidth > 767 && screenWidth <= 1035) {
      return Breakpoint.MEDIUM;
    } else {
      return Breakpoint.LARGE;
    }
  }

  private onScreenResize(): void {
    const newBreakpoint = RiskParametersView.getBreakpoint();
    if(newBreakpoint !== this.state.breakpoint) {
      this.setState({breakpoint: newBreakpoint});
    }
  }

  private onBuyingPowerChange(value: number) {
  	this.buyingPower = value;
  }

  private static STYLE = StyleSheet.create({
  	smallContainer: {
  		width: '60%',
  		minWidth: '320px',
  		maxWidth: '460px'
  	},
  	mediumContainer: {
  		width: '768px'
  	},
  	largeContainer: {
  		backgroundColor: 'pink'
  	}
  });

  private buyingPower: number;
}

class NumberInputBox extends React.Component<
	NumberInputBoxProperties, NumberInputBoxState> {
		constructor(props: NumberInputBoxProperties) {
			super(props);
			this.state = {
				value: ''
			}
			this.onChange = this.onChange.bind(this);
		}
		public render(): JSX.Element {
			return (
					<VBoxLayout width='100%'>
						<HBoxLayout width='100%'>
							<span className={css(NumberInputBox.STYLE.label)}>
								{this.props.label}
							</span>
							<Padding/>
						</HBoxLayout>
						<Padding size='12px'/>
						<input type='text' className={css(NumberInputBox.STYLE.input)}
							value={this.state.value} onChange={this.onChange}/>
					</VBoxLayout>
				)
		}

		private onChange(event: React.FormEvent<HTMLInputElement>) {
			const value = parseFloat(event.currentTarget.value);
			if(value && event.currentTarget.value.match('^[0-9]*[\\.]*[0-9]{0,2}$')) {
				let displayValue = value.toString();
				this.props.onChange(value);
				this.setState({value: event.currentTarget.value});
			} else if(event.currentTarget.value === '') {
				this.setState({value: ''});
				this.props.onChange(0);
			} else if(event.currentTarget.value.match('^[0]*\\.$')) {
				this.setState({value: '0.'});
				this.props.onChange(0);
			} else if(event.currentTarget.value.match('^[0]*$')) {
				this.setState({value: '0'});
				this.props.onChange(0);
			} else if(event.currentTarget.value.match('^[0]*\\.0$')) {
				this.setState({value: '0.0'});
				this.props.onChange(0);
			} else if(event.currentTarget.value.match('^[0]*\\.00$')) {
				this.setState({value: '0.00'});
				this.props.onChange(0);
			}
		}

		private static STYLE = StyleSheet.create({
			label: {
				font: '400 14px Roboto',
				color: '#333333',
      	whiteSpace: 'nowrap'
			},
			input: {
				width: '100%',
				font: '400 14px Roboto',
				color: '#333333'
			}
		});
}
class Footer extends React.Component<FooterProperties> {

}
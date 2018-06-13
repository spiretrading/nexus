import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';
import * as Beam from 'beam';

const model = new WebPortal.LocalLoginPageModel();
const onLogin = (account: Beam.DirectoryEntry) => {
	console.log('onLogin: ', account)
}
class App extends React.Component<{}> {
	public render(): JSX.Element {
		return (
				<div>
					<WebPortal.LoginPage model={model} onLogin={onLogin}/>
					<button onClick={() => model.accept.bind(model)()}>
						Accept
					</button>
					<button onClick={() => model.reject("Decided to reject")}>
						Reject
					</button>
				</div>
			);
	}
}
ReactDOM.render(<App/>, document.getElementById('main'));

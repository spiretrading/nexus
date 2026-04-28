import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

class TestApp extends React.Component {
  public render(): JSX.Element {
    return (
      <div style={STYLE.wrapper}>
        <WebPortal.GroupProfitAndLossController/>
      </div>);
  }
}

const STYLE: Record<string, React.CSSProperties> = {
  wrapper: {
    width: '100%',
    height: '100%',
    display: 'flex',
    flexDirection: 'column'
  }
};

ReactDOM.render(<TestApp/>, document.getElementById('main'));

import * as React from 'react';
import { ErrorMessage } from '../../../components/error_message';

interface Properties {

  /** The URL of the PDF on this origin, or a locally created blob URL. */
  filePath: string;

  /** The accessible title of the report viewer. */
  title: string;
}

interface State {
  filePath: string;
  failed: boolean;
  revision: number;
}

interface ViewerApplication {
  initializedPromise: Promise<void>;
  pdfLoadingTask?: {promise: Promise<unknown>};
  eventBus: {
    on(name: string, listener: () => void): void;
    off(name: string, listener: () => void): void;
  };
}

/** Displays a PDF using the separately loaded PDF.js viewer. */
export class PdfReportViewer extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {filePath: props.filePath, failed: false, revision: 0};
    this.frame = React.createRef();
    this.disconnect = null;
  }

  public render(): JSX.Element {
    if(!this.props.filePath) {
      return null;
    }
    const file = new URL(this.props.filePath, document.baseURI).href;
    const source =
      'resources/pdfjs/web/viewer.html?file=' + encodeURIComponent(file);
    return <div
        style={{
          width: '100%',
          height: '70vh',
          minHeight: '320px',
          contain: 'layout'
        }}>
      {(() => {
        if(this.state.filePath === this.props.filePath && this.state.failed) {
          return <ErrorMessage message='There was an error loading the report.'
            onRetry={this.onRetry}/>;
        }
        return <iframe key={file + ':' + this.state.revision} ref={this.frame}
          src={source} title={this.props.title} onLoad={this.onLoad}
          style={{
            display: 'block',
            width: '100%',
            height: '100%',
            border: 0
          }}/>;
      })()}
    </div>;
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.filePath !== this.props.filePath) {
      this.disconnect?.();
      this.disconnect = null;
      this.setState({filePath: this.props.filePath, failed: false});
    }
  }

  public componentWillUnmount(): void {
    this.disconnect?.();
  }

  private onLoad = async () => {
    this.disconnect?.();
    this.disconnect = null;
    const frame = this.frame.current;
    if(!frame) {
      return;
    }
    const window = frame.contentWindow as Window & {
      PDFViewerApplication?: ViewerApplication;
    };
    const fail = () => {
      if(this.frame.current === frame) {
        this.setState({failed: true});
      }
    };
    try {
      const viewer = window.PDFViewerApplication;
      if(!viewer) {
        fail();
        return;
      }
      await viewer.initializedPromise;
      if(this.frame.current !== frame) {
        return;
      }
      viewer.eventBus.on('documenterror', fail);
      this.disconnect = () => viewer.eventBus.off('documenterror', fail);
      await viewer.pdfLoadingTask?.promise;
    } catch {
      fail();
    }
  };

  private onRetry = () => {
    this.disconnect?.();
    this.disconnect = null;
    this.setState(state => ({failed: false, revision: state.revision + 1}));
  };

  private frame: React.RefObject<HTMLIFrameElement>;
  private disconnect: () => void;
}

import * as React from 'react';
import { ErrorMessage } from '../../../components/error_message';

interface Properties {

  /** The URL of the PDF on this origin, or a locally created blob URL. */
  filePath: string;

  /** The accessible title of the report viewer. */
  title: string;
}

interface State {
  failed: boolean;
}

interface Viewer {
  ready: Promise<void>;
  dispose(): void;
}

interface ViewerRuntime {
  create(container: HTMLElement, file: string, onError: () => void): Viewer;
}

/** Displays PDF pages using a separately loaded PDF.js renderer. */
export class PdfReportViewer extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {failed: false};
    this.container = React.createRef();
    this.viewer = null;
    this.generation = 0;
  }

  public render(): JSX.Element {
    if(!this.props.filePath) {
      return null;
    }
    return <div
        style={{
          position: 'relative',
          width: '100%',
          height: '70vh',
          minHeight: '320px',
          contain: 'layout'
        }}>
      {(() => {
        if(this.state.failed) {
          return <ErrorMessage message='There was an error loading the report.'
            onRetry={this.onRetry}/>;
        }
        return <div ref={this.container} className='spire-pdf-report'
          role='region' aria-label={this.props.title} tabIndex={0}
          style={{
            position: 'absolute',
            inset: 0,
            overflow: 'auto'
          }}/>;
      })()}
    </div>;
  }

  public componentDidMount(): void {
    this.load();
  }

  public componentDidUpdate(previous: Properties): void {
    if(previous.filePath !== this.props.filePath) {
      this.load();
    }
  }

  public componentWillUnmount(): void {
    ++this.generation;
    this.viewer?.dispose();
  }

  private load(): void {
    const generation = ++this.generation;
    this.viewer?.dispose();
    this.viewer = null;
    this.setState({failed: false}, () => this.initialize(generation));
  }

  private async initialize(generation: number): Promise<void> {
    if(generation !== this.generation || !this.props.filePath) {
      return;
    }
    const fail = () => {
      if(generation === this.generation) {
        this.viewer?.dispose();
        this.viewer = null;
        this.setState({failed: true});
      }
    };
    try {
      const file = new URL(this.props.filePath, document.baseURI).href;
      const runtime = await loadRuntime();
      if(generation !== this.generation) {
        return;
      }
      this.viewer = runtime.create(this.container.current, file, fail);
      await this.viewer.ready;
    } catch {
      fail();
    }
  }

  private onRetry = () => {
    this.load();
  };

  private container: React.RefObject<HTMLDivElement>;
  private viewer: Viewer;
  private generation: number;
}

function loadRuntime(): Promise<ViewerRuntime> {
  if(!runtimePromise) {
    const styles = ['resources/pdfjs/web/pdf_viewer.css',
      'resources/report_viewer/viewer.css'].map(source => {
      const link = document.createElement('link');
      link.rel = 'stylesheet';
      link.href = source;
      return link;
    });
    const script = document.createElement('script');
    script.src = 'resources/report_viewer/viewer.js?attempt=' + runtimeAttempt++;
    const elements = [...styles, script];
    runtimePromise = Promise.all(elements.map(element =>
      new Promise<void>((resolve, reject) => {
        element.onload = () => resolve();
        element.onerror = () => reject(new Error('Unable to load PDF viewer.'));
        document.head.appendChild(element);
      }))).then(() => {
        const runtime = (window as Window & {
          spirePdfViewerReady?: Promise<ViewerRuntime>;
        }).spirePdfViewerReady;
        if(!runtime) {
          throw new Error('Unable to initialize PDF viewer.');
        }
        return runtime;
      }).catch(error => {
        for(const element of elements) {
          element.remove();
        }
        runtimePromise = null;
        throw error;
      });
  }
  return runtimePromise;
}

let runtimePromise: Promise<ViewerRuntime>;
let runtimeAttempt = 0;

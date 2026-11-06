import * as React from 'react';
import type { ReportViewer } from './report_viewer';
import { ErrorMessage } from '../../../components/error_message';

interface Properties {

  /** The URL of the CSV on this origin, or a locally created blob URL. */
  filePath: string;

  /** The accessible title of the report viewer. */
  title: string;
}

interface State {
  failed: boolean;
}

/** Displays a CSV report using a separately loaded data grid. */
export class CsvReportViewer extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      failed: false
    };
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
        return <div ref={this.container} className='spire-csv-report'
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

  private static loadRuntime(): Promise<ReportViewer.Runtime> {
    if(!CsvReportViewer.runtimePromise) {
      const script = document.createElement('script');
      script.src = 'resources/csv_report_viewer/viewer.js?attempt=' +
        CsvReportViewer.runtimeAttempt;
      ++CsvReportViewer.runtimeAttempt;
      CsvReportViewer.runtimePromise = new Promise<void>((resolve, reject) => {
        script.onload = () => resolve();
        script.onerror = () => reject(new Error('Unable to load CSV viewer.'));
        document.head.appendChild(script);
      }).then(() => {
        const runtime = (window as Window & {
          spireCsvViewerReady?: Promise<ReportViewer.Runtime>;
        }).spireCsvViewerReady;
        if(!runtime) {
          throw new Error('Unable to initialize CSV viewer.');
        }
        return runtime;
      }).catch(error => {
        script.remove();
        CsvReportViewer.runtimePromise = null;
        throw error;
      });
    }
    return CsvReportViewer.runtimePromise;
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
      const runtime = await CsvReportViewer.loadRuntime();
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

  private static runtimePromise: Promise<ReportViewer.Runtime>;
  private static runtimeAttempt = 0;
  private container: React.RefObject<HTMLDivElement>;
  private viewer: ReportViewer.Instance;
  private generation: number;
}

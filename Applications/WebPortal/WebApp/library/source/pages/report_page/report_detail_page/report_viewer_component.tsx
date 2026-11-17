import * as React from 'react';
import type { ReportViewer } from './report_viewer';
import type { ReportViewerRuntimeLoader } from './report_viewer_runtime_loader';
import { ErrorMessage } from '../../../components/error_message';

interface State {
  failed: boolean;
}

/** Manages the loading, display, and disposal of a report viewer. */
export abstract class ReportViewerComponent extends
    React.Component<ReportViewerComponent.Properties, State> {

  /**
   * Creates a report viewer component.
   * @param props The report file and accessible title.
   * @param loader Loads the runtime used to display the report.
   * @param className The CSS class for the runtime's container.
   */
  constructor(props: ReportViewerComponent.Properties,
      loader: ReportViewerRuntimeLoader, className: string) {
    super(props);
    this.state = {
      failed: false
    };
    this.loader = loader;
    this.className = className;
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
        return <div ref={this.container} className={this.className}
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

  public componentDidUpdate(
      previous: ReportViewerComponent.Properties): void {
    if(previous.filePath !== this.props.filePath) {
      this.load();
    }
  }

  public componentWillUnmount(): void {
    ++this.generation;
    this.viewer?.dispose();
  }

  private load(): void {
    ++this.generation;
    const generation = this.generation;
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
      const runtime = await this.loader.load();
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

  private loader: ReportViewerRuntimeLoader;
  private className: string;
  private container: React.RefObject<HTMLDivElement>;
  private viewer: ReportViewer.Instance;
  private generation: number;
}

export namespace ReportViewerComponent {

  /** Identifies the report file and its accessible title. */
  export interface Properties {

    /** The report file's URL on this origin, or a locally created blob URL. */
    filePath: string;

    /** The accessible title of the report viewer. */
    title: string;
  }
}

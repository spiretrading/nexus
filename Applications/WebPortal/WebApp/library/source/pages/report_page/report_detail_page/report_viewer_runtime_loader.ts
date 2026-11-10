import type { ReportViewer } from './report_viewer';

/** Loads and caches a report viewer runtime, allowing retries after failure. */
export class ReportViewerRuntimeLoader {

  /**
   * Creates a runtime loader without loading any resources.
   * @param script The URL of the runtime's script.
   * @param styles The URLs of the runtime's stylesheets.
   * @param getRuntime Returns the runtime published by the loaded script.
   */
  constructor(script: string, styles: string[],
      getRuntime: () => Promise<ReportViewer.Runtime>) {
    this.script = script;
    this.styles = [...styles];
    this.getRuntime = getRuntime;
    this.runtimePromise = null;
    this.runtimeAttempt = 0;
  }

  /**
   * Loads the resources once, or retries a previously failed load.
   * @return The shared runtime promise.
   */
  public load(): Promise<ReportViewer.Runtime> {
    if(!this.runtimePromise) {
      const styles = this.styles.map(source => {
        const link = document.createElement('link');
        link.rel = 'stylesheet';
        link.href = source;
        return link;
      });
      const script = document.createElement('script');
      const source = new URL(this.script, document.baseURI);
      source.searchParams.set('attempt', String(this.runtimeAttempt));
      ++this.runtimeAttempt;
      script.src = source.href;
      const elements = [...styles, script];
      this.runtimePromise = Promise.all(elements.map(element =>
        new Promise<void>((resolve, reject) => {
          element.onload = () => resolve();
          element.onerror = () =>
            reject(new Error('Unable to load report viewer.'));
          document.head.appendChild(element);
        }))).then(() => this.getRuntime()).then(runtime => {
          if(!runtime) {
            throw new Error('Unable to initialize report viewer.');
          }
          return runtime;
        }).catch(error => {
          for(const element of elements) {
            element.remove();
          }
          this.runtimePromise = null;
          throw error;
        });
    }
    return this.runtimePromise;
  }

  private script: string;
  private styles: string[];
  private getRuntime: () => Promise<ReportViewer.Runtime>;
  private runtimePromise: Promise<ReportViewer.Runtime>;
  private runtimeAttempt: number;
}

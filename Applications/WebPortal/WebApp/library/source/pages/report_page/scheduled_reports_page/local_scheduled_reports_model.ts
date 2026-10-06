import { ScheduledReportsModel } from './scheduled_reports_model';

/** Stores scheduled reports and immediate run requests in memory. */
export class LocalScheduledReportsModel extends ScheduledReportsModel {

  /** Constructs a local model.
   * @param schedules - The initial scheduled reports.
   */
  constructor(schedules: readonly ScheduledReportsModel.Schedule[]) {
    super();
    this.loaded = false;
    this.entries = new Map();
    for(const schedule of schedules) {
      this.entries.set(schedule.id, copySchedule(schedule));
    }
    this.requests = [];
    this.nextId = 1;
  }

  /** Returns whether this model has been loaded. */
  public get isLoaded(): boolean {
    return this.loaded;
  }

  /** Returns copies of the scheduled reports, in display order. */
  public get schedules(): readonly ScheduledReportsModel.Schedule[] {
    this.ensureLoaded();
    return Array.from(this.entries.values(), copySchedule);
  }

  /** Returns snapshots of reports requested through Run Now, in order. */
  public get runs(): readonly ScheduledReportsModel.Schedule[] {
    this.ensureLoaded();
    return this.requests.map(copySchedule);
  }

  /** Adds a scheduled report at the front of the list.
   * @return The identifier assigned to the schedule.
   */
  public add(schedule: Omit<ScheduledReportsModel.Schedule, 'id'>): string {
    this.ensureLoaded();
    while(this.entries.has(String(this.nextId))) {
      ++this.nextId;
    }
    const id = String(this.nextId++);
    this.entries = new Map([[id, copySchedule({...schedule, id})],
      ...this.entries]);
    return id;
  }

  /** Updates a schedule's display values while preserving its list position. */
  public update(schedule: ScheduledReportsModel.Schedule): void {
    this.ensureLoaded();
    this.find(schedule.id);
    this.entries.set(schedule.id, copySchedule(schedule));
  }

  public async load(): Promise<void> {
    this.loaded = true;
  }

  public async query(submission: ScheduledReportsModel.Submission):
      Promise<ScheduledReportsModel.Response> {
    this.ensureLoaded();
    if(!Number.isSafeInteger(submission.pageIndex) ||
        submission.pageIndex < 0) {
      throw new Error('Invalid page index.');
    }
    const query = submission.filters.query.toLowerCase();
    const matches = Array.from(this.entries.values()).filter(schedule => {
      const title = (() => {
        if(schedule.repeats) {
          return `Recurring ${schedule.type}`;
        }
        return schedule.type;
      })();
      return [title, schedule.runDate.label,
        ...schedule.parameters.flatMap(parameter =>
          [parameter.label, parameter.value])].some(value =>
        value.toLowerCase().includes(query));
    });
    const start = submission.pageIndex * ScheduledReportsModel.PAGE_SIZE;
    return {
      status: ScheduledReportsModel.ResponseStatus.READY,
      isEmpty: this.entries.size === 0,
      filteredCount: matches.length,
      schedules: matches.slice(start, start + ScheduledReportsModel.PAGE_SIZE).
        map(copySchedule)
    };
  }

  public async run(id: string): Promise<void> {
    this.ensureLoaded();
    this.requests.push(copySchedule(this.find(id)));
  }

  public async duplicate(id: string): Promise<ScheduledReportsModel.Schedule> {
    this.ensureLoaded();
    const schedule = copySchedule(this.find(id));
    while(this.entries.has(String(this.nextId))) {
      ++this.nextId;
    }
    schedule.id = String(this.nextId++);
    const entries = Array.from(this.entries);
    const index = entries.findIndex(([key]) => key === id);
    entries.splice(index + 1, 0, [schedule.id, schedule]);
    this.entries = new Map(entries);
    return copySchedule(schedule);
  }

  public async delete(id: string): Promise<void> {
    this.ensureLoaded();
    this.find(id);
    this.entries.delete(id);
  }

  private ensureLoaded(): void {
    if(!this.isLoaded) {
      throw new Error('Model not loaded.');
    }
  }

  private find(id: string): ScheduledReportsModel.Schedule {
    const schedule = this.entries.get(id);
    if(!schedule) {
      throw new Error(`Scheduled report not found: ${id}`);
    }
    return schedule;
  }

  private loaded: boolean;
  private entries: Map<string, ScheduledReportsModel.Schedule>;
  private requests: ScheduledReportsModel.Schedule[];
  private nextId: number;
}

function copySchedule(schedule: ScheduledReportsModel.Schedule):
    ScheduledReportsModel.Schedule {
  return {
    ...schedule,
    parameters: schedule.parameters.map(parameter => ({...parameter})),
    runDate: {...schedule.runDate}
  };
}

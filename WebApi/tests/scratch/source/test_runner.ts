(global as any).window = {
  document: {}
};
import {TestSet, TestRunner} from "alsatian";

let testSet = TestSet.create();
testSet.addTestsFromFiles("./application/*.js");
let testRunner = new TestRunner();
testRunner.outputStream.pipe(process.stdout);
testRunner.run(testSet);

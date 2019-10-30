/*
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
*/

import React from "react";
import { PageSection, PageSectionVariants } from "@patternfly/react-core";
import { Stack, StackItem } from "@patternfly/react-core";
import { Split, SplitItem } from "@patternfly/react-core";

import EntityListTable from "./entityListTable";
import EntityList from "./entityList";
import RouterSelect from "./routerSelect";
import Updated from "../updated";

class EntitiesPage extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      loading: false,
      lastUpdated: new Date(),
      entity: null,
      routerId: null
    };
    this.schema = this.props.service.management.schema();
  }

  lastUpdated = lastUpdated => {
    this.setState({ lastUpdated });
  };

  handleSelectEntity = entity => {
    this.setState({ entity });
  };

  handleRouterSelected = routerId => {
    this.setState({ routerId });
  };

  render() {
    return (
      <PageSection
        variant={PageSectionVariants.light}
        className="details-table-page"
      >
        <Stack>
          <StackItem className="details-header pf-u-box-shadow-sm-bottom">
            <Split>
              <SplitItem isFilled className="split-left">
                <span className="prompt">Router</span>{" "}
                <RouterSelect
                  service={this.props.service}
                  handleRouterSelected={this.handleRouterSelected}
                />
              </SplitItem>
              <SplitItem>
                <Updated
                  service={this.props.service}
                  lastUpdated={this.state.lastUpdated}
                />
              </SplitItem>
            </Split>
          </StackItem>
          <StackItem className="details-table">
            <Split>
              <SplitItem id="entityList">
                <EntityList
                  schema={this.schema}
                  handleSelectEntity={this.handleSelectEntity}
                />
              </SplitItem>
              <SplitItem isFilled>
                {this.state.entity ? (
                  <EntityListTable
                    {...this.props}
                    entity={this.state.entity}
                    schema={this.schema}
                    routerId={this.state.routerId}
                    lastUpdated={this.lastUpdated}
                  />
                ) : (
                  <React.Fragment />
                )}
              </SplitItem>
            </Split>
          </StackItem>
        </Stack>
      </PageSection>
    );
  }
}

export default EntitiesPage;

import React from "react";
import {
  Table,
  TableHeader,
  TableBody,
  textCenter
} from "@patternfly/react-table";

class ActiveAddressesCard extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      columns: [
        { title: "Repositories" },
        "Branches",
        { title: "Pull requests" },
        "Workspaces",
        {
          title: "Last Commit",
          transforms: [textCenter],
          cellTransforms: [textCenter]
        }
      ],
      rows: [
        {
          cells: ["one", "two", "three", "four", "five"]
        },
        {
          cells: [
            {
              title: <div>one - 2</div>,
              props: { title: "hover title", colSpan: 3 }
            },
            "four - 2",
            "five - 2"
          ]
        },
        {
          cells: [
            "one - 3",
            "two - 3",
            "three - 3",
            "four - 3",
            {
              title: "five - 3 (not centered)",
              props: { textCenter: false }
            }
          ]
        }
      ]
    };
  }

  render() {
    const { columns, rows } = this.state;

    return (
      <Table caption="Simple Table" cells={columns} rows={rows}>
        <TableHeader />
        <TableBody />
      </Table>
    );
  }
}

export default ActiveAddressesCard;
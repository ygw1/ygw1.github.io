package com.quadtalent.quadx.outputEntity;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.io.Serializable;
import java.util.List;

@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class Output implements Serializable {
    private static final long serialVersionUID = -8703416493289824090L;
    private String estimateCode;
    private List<List<LoadTruck>> solutionArray;
}

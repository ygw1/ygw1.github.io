package com.quadtalent.quadx.inputEntity;

import com.fasterxml.jackson.annotation.JsonProperty;
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
public class RawInput implements Serializable {
    private String estimateCode;

    @JsonProperty("algorithmBaseParamDto")
    private AlgorithmBaseParamDto env;
    private List<Box> boxes;
}
